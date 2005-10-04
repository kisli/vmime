//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/smtp/SMTPTransport.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"
#include "vmime/encoderB64.hpp"
#include "vmime/mailboxList.hpp"

#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"

#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/SASLContext.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/net/tls/TLSSession.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(getInfos().hasProperty(getSession(), \
		dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace smtp {


SMTPTransport::SMTPTransport(ref <session> sess, ref <security::authenticator> auth, const bool secured)
	: transport(sess, getInfosInstance(), auth), m_socket(NULL),
	  m_authentified(false), m_extendedSMTP(false), m_timeoutHandler(NULL),
	  m_secured(secured)
{
}


SMTPTransport::~SMTPTransport()
{
	try
	{
		if (isConnected())
			disconnect();
		else if (m_socket)
			internalDisconnect();
	}
	catch (vmime::exception&)
	{
		// Ignore
	}
}


const string SMTPTransport::getProtocolName() const
{
	return "smtp";
}


void SMTPTransport::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	// Create the time-out handler
	if (HAS_PROPERTY(PROPERTY_TIMEOUT_FACTORY))
	{
		timeoutHandlerFactory* tof = platformDependant::getHandler()->
			getTimeoutHandlerFactory(GET_PROPERTY(string, PROPERTY_TIMEOUT_FACTORY));

		m_timeoutHandler = tof->create();
	}

	// Create and connect the socket
	m_socket = getSocketFactory()->create();

#if VMIME_HAVE_TLS_SUPPORT
	if (m_secured)  // dedicated port/SMTPS
	{
		ref <tls::TLSSession> tlsSession =
			vmime::create <tls::TLSSession>(getCertificateVerifier());

		ref <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		m_socket = tlsSocket;
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	m_socket->connect(address, port);

	m_responseBuffer.clear();

	// Connection
	//
	// eg:  C: <connection to server>
	// ---  S: 220 smtp.domain.com Service ready

	string response;

	if (readAllResponses(response) != 220)
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(response);
	}

	// Identification
	// First, try Extended SMTP (ESMTP)
	//
	// eg:  C: EHLO thismachine.ourdomain.com
	//      S: 250-smtp.theserver.com
	//      S: 250 AUTH CRAM-MD5 DIGEST-MD5

	sendRequest("EHLO " + platformDependant::getHandler()->getHostName());

	if (readAllResponses(response, true) != 250)
	{
		// Next, try "Basic" SMTP
		//
		// eg:  C: HELO thismachine.ourdomain.com
		//      S: 250 OK

		sendRequest("HELO " + platformDependant::getHandler()->getHostName());

		if (readAllResponses(response) != 250)
		{
			internalDisconnect();
			throw exceptions::connection_greeting_error(response);
		}

		m_extendedSMTP = false;
	}
	else
	{
		m_extendedSMTP = true;
		m_extendedSMTPResponse = response;
	}

#if VMIME_HAVE_TLS_SUPPORT
	// Setup secured connection, if requested
	const bool tls = HAS_PROPERTY(PROPERTY_CONNECTION_TLS)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS);
	const bool tlsRequired = HAS_PROPERTY(PROPERTY_CONNECTION_TLS_REQUIRED)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS_REQUIRED);

	if (!m_secured && tls)  // only if not POP3S
	{
		try
		{
			startTLS();
		}
		// Non-fatal error
		catch (exceptions::command_error&)
		{
			if (tlsRequired)
			{
				throw;
			}
			else
			{
				// TLS is not required, so don't bother
			}
		}
		// Fatal error
		catch (...)
		{
			throw;
		}
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	// Authentication
	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_NEEDAUTH))
		authenticate();
}


void SMTPTransport::authenticate()
{
	if (!m_extendedSMTP)
	{
		internalDisconnect();
		throw exceptions::command_error("AUTH", "ESMTP not supported.");
	}

	getAuthenticator()->setService(thisRef().dynamicCast <service>());

#if VMIME_HAVE_SASL_SUPPORT
	// First, try SASL authentication
	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_SASL))
	{
		try
		{
			authenticateSASL();

			m_authentified = true;
			return;
		}
		catch (exceptions::authentication_error& e)
		{
			if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_SASL_FALLBACK))
			{
				// Can't fallback on normal authentication
				internalDisconnect();
				throw e;
			}
			else
			{
				// Ignore, will try normal authentication
			}
		}
		catch (exception& e)
		{
			internalDisconnect();
			throw e;
		}
	}
#endif // VMIME_HAVE_SASL_SUPPORT

	// No other authentication method is possible
	throw exceptions::authentication_error("All authentication methods failed");
}


#if VMIME_HAVE_SASL_SUPPORT

void SMTPTransport::authenticateSASL()
{
	if (!getAuthenticator().dynamicCast <security::sasl::SASLAuthenticator>())
		throw exceptions::authentication_error("No SASL authenticator available.");

	// Obtain SASL mechanisms supported by server from EHLO response
	std::vector <string> saslMechs;
	std::istringstream iss(m_extendedSMTPResponse);

	while (!iss.eof())
	{
		string line;
		std::getline(iss, line);

		std::istringstream liss(line);
		string word;

		bool inAuth = false;

		while (liss >> word)
		{
			if (word.length() == 4 &&
			    (word[0] == 'A' || word[0] == 'a') ||
			    (word[0] == 'U' || word[0] == 'u') ||
			    (word[0] == 'T' || word[0] == 't') ||
			    (word[0] == 'H' || word[0] == 'h'))
			{
				inAuth = true;
			}
			else if (inAuth)
			{
				saslMechs.push_back(word);
			}
		}
	}

	if (saslMechs.empty())
		throw exceptions::authentication_error("No SASL mechanism available.");

	std::vector <ref <security::sasl::SASLMechanism> > mechList;

	ref <security::sasl::SASLContext> saslContext =
		vmime::create <security::sasl::SASLContext>();

	for (unsigned int i = 0 ; i < saslMechs.size() ; ++i)
	{
		try
		{
			mechList.push_back
				(saslContext->createMechanism(saslMechs[i]));
		}
		catch (exceptions::no_such_mechanism&)
		{
			// Ignore mechanism
		}
	}

	if (mechList.empty())
		throw exceptions::authentication_error("No SASL mechanism available.");

	// Try to suggest a mechanism among all those supported
	ref <security::sasl::SASLMechanism> suggestedMech =
		saslContext->suggestMechanism(mechList);

	if (!suggestedMech)
		throw exceptions::authentication_error("Unable to suggest SASL mechanism.");

	// Allow application to choose which mechanisms to use
	mechList = getAuthenticator().dynamicCast <security::sasl::SASLAuthenticator>()->
		getAcceptableMechanisms(mechList, suggestedMech);

	if (mechList.empty())
		throw exceptions::authentication_error("No SASL mechanism available.");

	// Try each mechanism in the list in turn
	for (unsigned int i = 0 ; i < mechList.size() ; ++i)
	{
		ref <security::sasl::SASLMechanism> mech = mechList[i];

		ref <security::sasl::SASLSession> saslSession =
			saslContext->createSession("smtp", getAuthenticator(), mech);

		saslSession->init();

		sendRequest("AUTH " + mech->getName());

		for (bool cont = true ; cont ; )
		{
			string response;

			switch (readAllResponses(response))
			{
			case 235:
			{
				m_socket = saslSession->getSecuredSocket(m_socket);
				return;
			}
			case 334:
			{
				byte* challenge = 0;
				int challengeLen = 0;

				byte* resp = 0;
				int respLen = 0;

				try
				{
					// Extract challenge
					saslContext->decodeB64(response, &challenge, &challengeLen);

					// Prepare response
					saslSession->evaluateChallenge
						(challenge, challengeLen, &resp, &respLen);

					// Send response
					sendRequest(saslContext->encodeB64(resp, respLen));
				}
				catch (exceptions::sasl_exception& e)
				{
					if (challenge)
					{
						delete [] challenge;
						challenge = NULL;
					}

					if (resp)
					{
						delete [] resp;
						resp = NULL;
					}

					// Cancel SASL exchange
					sendRequest("*");
				}
				catch (...)
				{
					if (challenge)
						delete [] challenge;

					if (resp)
						delete [] resp;

					throw;
				}

				if (challenge)
					delete [] challenge;

				if (resp)
					delete [] resp;

				break;
			}
			default:

				cont = false;
				break;
			}
		}
	}

	throw exceptions::authentication_error
		("Could not authenticate using SASL: all mechanisms failed.");
}

#endif // VMIME_HAVE_SASL_SUPPORT


#if VMIME_HAVE_TLS_SUPPORT

void SMTPTransport::startTLS()
{
	try
	{
		sendRequest("STARTTLS");

		string response;

		if (readAllResponses(response) != 220)
			throw exceptions::command_error("STARTTLS", response);

		ref <tls::TLSSession> tlsSession =
			vmime::create <tls::TLSSession>(getCertificateVerifier());

		ref <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		tlsSocket->handshake(m_timeoutHandler);

		m_socket = tlsSocket;
	}
	catch (exceptions::command_error&)
	{
		// Non-fatal error
		throw;
	}
	catch (exception&)
	{
		// Fatal error
		internalDisconnect();
		throw;
	}
}

#endif // VMIME_HAVE_TLS_SUPPORT


const bool SMTPTransport::isConnected() const
{
	return (m_socket && m_socket->isConnected() && m_authentified);
}


void SMTPTransport::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void SMTPTransport::internalDisconnect()
{
	sendRequest("QUIT");

	m_socket->disconnect();
	m_socket = NULL;

	m_timeoutHandler = NULL;

	m_authentified = false;
	m_extendedSMTP = false;
}


void SMTPTransport::noop()
{
	sendRequest("NOOP");

	string response;

	if (readAllResponses(response) != 250)
		throw exceptions::command_error("NOOP", response);
}


void SMTPTransport::send(const mailbox& expeditor, const mailboxList& recipients,
                         utility::inputStream& is, const utility::stream::size_type size,
                         utility::progressionListener* progress)
{
	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty())
		throw exceptions::no_recipient();
	else if (expeditor.isEmpty())
		throw exceptions::no_expeditor();

	// Emit the "MAIL" command
	string response;

	sendRequest("MAIL FROM: <" + expeditor.getEmail() + ">");

	if (readAllResponses(response) != 250)
	{
		internalDisconnect();
		throw exceptions::command_error("MAIL", response);
	}

	// Emit a "RCPT TO" command for each recipient
	for (int i = 0 ; i < recipients.getMailboxCount() ; ++i)
	{
		const mailbox& mbox = *recipients.getMailboxAt(i);

		sendRequest("RCPT TO: <" + mbox.getEmail() + ">");

		if (readAllResponses(response) != 250)
		{
			internalDisconnect();
			throw exceptions::command_error("RCPT TO", response);
		}
	}

	// Send the message data
	sendRequest("DATA");

	if (readAllResponses(response) != 354)
	{
		internalDisconnect();
		throw exceptions::command_error("DATA", response);
	}

	// Stream copy with "\n." to "\n.." transformation
	utility::outputStreamSocketAdapter sos(*m_socket);
	utility::dotFilteredOutputStream fos(sos);

	utility::bufferedStreamCopy(is, fos, size, progress);

	// Send end-of-data delimiter
	m_socket->sendRaw("\r\n.\r\n", 5);

	if (readAllResponses(response) != 250)
	{
		internalDisconnect();
		throw exceptions::command_error("DATA", response);
	}
}


void SMTPTransport::sendRequest(const string& buffer, const bool end)
{
	m_socket->send(buffer);
	if (end) m_socket->send("\r\n");
}


const int SMTPTransport::getResponseCode(const string& response)
{
	int code = 0;

	if (response.length() >= 3)
	{
		code = (response[0] - '0') * 100
		     + (response[1] - '0') * 10
		     + (response[2] - '0');
	}

	return (code);
}


const string SMTPTransport::readResponseLine()
{
	string currentBuffer = m_responseBuffer;

	while (true)
	{
		// Get a line from the response buffer
		string::size_type lineEnd = currentBuffer.find_first_of('\n');

		if (lineEnd != string::npos)
		{
			const string line(currentBuffer.begin(), currentBuffer.begin() + lineEnd);

			currentBuffer.erase(currentBuffer.begin(), currentBuffer.begin() + lineEnd + 1);
			m_responseBuffer = currentBuffer;

			return line;
		}

		// Check whether the time-out delay is elapsed
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
				throw exceptions::operation_timed_out();

			m_timeoutHandler->resetTimeOut();
		}

		// Receive data from the socket
		string receiveBuffer;
		m_socket->receive(receiveBuffer);

		if (receiveBuffer.empty())   // buffer is empty
		{
			platformDependant::getHandler()->wait();
			continue;
		}

		currentBuffer += receiveBuffer;
	}
}


const int SMTPTransport::readResponse(string& text)
{
	string line = readResponseLine();

	// Special case where CRLF occurs after response code
	if (line.length() < 4)
		line = line + '\n' + readResponseLine();

	const int code = getResponseCode(line);

	m_responseContinues = (line.length() >= 4 && line[3] == '-');

	if (line.length() > 4)
		text = utility::stringUtils::trim(line.substr(4));
	else
		text = utility::stringUtils::trim(line);

	return code;
}


const int SMTPTransport::readAllResponses(string& outText, const bool allText)
{
	string text;

	const int firstCode = readResponse(outText);

	if (allText)
		text = outText;

	while (m_responseContinues)
	{
		const int code = readResponse(outText);

		if (allText)
			text += '\n' + outText;

		if (code != firstCode)
		{
			if (allText)
				outText = text;

			return 0;
		}
	}

	if (allText)
		outText = text;

	return firstCode;
}



// Service infos

SMTPServiceInfos SMTPTransport::sm_infos(false);


const serviceInfos& SMTPTransport::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& SMTPTransport::getInfos() const
{
	return sm_infos;
}


} // smtp
} // net
} // vmime
