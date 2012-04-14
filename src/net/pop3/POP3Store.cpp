//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3Folder.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"
#include "vmime/messageId.hpp"
#include "vmime/security/digest/messageDigestFactory.hpp"
#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/inputStreamSocketAdapter.hpp"

#include "vmime/net/defaultConnectionInfos.hpp"

#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/SASLContext.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/net/tls/TLSSession.hpp"
	#include "vmime/net/tls/TLSSecuredConnectionInfos.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT

#include <algorithm>


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const POP3ServiceInfos&>(getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(getInfos().hasProperty(getSession(), \
		dynamic_cast <const POP3ServiceInfos&>(getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace pop3 {


POP3Store::POP3Store(ref <session> sess, ref <security::authenticator> auth, const bool secured)
	: store(sess, getInfosInstance(), auth), m_socket(NULL),
	  m_authentified(false), m_timeoutHandler(NULL),
	  m_isPOP3S(secured), m_secured(false)
{
}


POP3Store::~POP3Store()
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


const string POP3Store::getProtocolName() const
{
	return "pop3";
}


ref <folder> POP3Store::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(folder::path(folder::path::component("INBOX")),
		thisRef().dynamicCast <POP3Store>());
}


ref <folder> POP3Store::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(folder::path(),
		thisRef().dynamicCast <POP3Store>());
}


ref <folder> POP3Store::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(path,
		thisRef().dynamicCast <POP3Store>());
}


bool POP3Store::isValidFolderName(const folder::path::component& /* name */) const
{
	return true;
}


void POP3Store::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	// Create the time-out handler
	if (getTimeoutHandlerFactory())
		m_timeoutHandler = getTimeoutHandlerFactory()->create();

	// Create and connect the socket
	m_socket = getSocketFactory()->create(m_timeoutHandler);

#if VMIME_HAVE_TLS_SUPPORT
	if (m_isPOP3S)  // dedicated port/POP3S
	{
		ref <tls::TLSSession> tlsSession =
			vmime::create <tls::TLSSession>(getCertificateVerifier());

		ref <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		m_socket = tlsSocket;

		m_secured = true;
		m_cntInfos = vmime::create <tls::TLSSecuredConnectionInfos>(address, port, tlsSession, tlsSocket);
	}
	else
#endif // VMIME_HAVE_TLS_SUPPORT
	{
		m_cntInfos = vmime::create <defaultConnectionInfos>(address, port);
	}

	m_socket->connect(address, port);

	// Connection
	//
	// eg:  C: <connection to server>
	// ---  S: +OK MailSite POP3 Server 5.3.4.0 Ready <36938848.1056800841.634@somewhere.com>

	string response;
	readResponse(response, false);

	if (!isSuccessResponse(response))
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(response);
	}

#if VMIME_HAVE_TLS_SUPPORT
	// Setup secured connection, if requested
	const bool tls = HAS_PROPERTY(PROPERTY_CONNECTION_TLS)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS);
	const bool tlsRequired = HAS_PROPERTY(PROPERTY_CONNECTION_TLS_REQUIRED)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS_REQUIRED);

	if (!m_isPOP3S && tls)  // only if not POP3S
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

	// Start authentication process
	authenticate(messageId(response));
}


void POP3Store::authenticate(const messageId& randomMID)
{
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
				// Can't fallback on APOP/normal authentication
				internalDisconnect();
				throw e;
			}
			else
			{
				// Ignore, will try APOP/normal authentication
			}
		}
		catch (exception& e)
		{
			internalDisconnect();
			throw e;
		}
	}
#endif // VMIME_HAVE_SASL_SUPPORT

	// Secured authentication with APOP (if requested and if available)
	//
	// eg:  C: APOP vincent <digest>
	// ---  S: +OK vincent is a valid mailbox

	const string username = getAuthenticator()->getUsername();
	const string password = getAuthenticator()->getPassword();

	string response;

	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP))
	{
		if (randomMID.getLeft().length() != 0 &&
		    randomMID.getRight().length() != 0)
		{
			// <digest> is the result of MD5 applied to "<message-id>password"
			ref <security::digest::messageDigest> md5 =
				security::digest::messageDigestFactory::getInstance()->create("md5");

			md5->update(randomMID.generate() + password);
			md5->finalize();

			sendRequest("APOP " + username + " " + md5->getHexDigest());
			readResponse(response, false);

			if (isSuccessResponse(response))
			{
				m_authentified = true;
				return;
			}
			else
			{
				// Some servers close the connection after an unsuccessful APOP
				// command, so the fallback may not always work...
				//
				// S: +OK Qpopper (version 4.0.5) at xxx starting.  <30396.1126730747@xxx>
				// C: APOP plop c5e0a87d088ec71d60e32692d4c5bdf4
				// S: -ERR [AUTH] Password supplied for "plop" is incorrect.
				// S: +OK Pop server at xxx signing off.
				// [Connection closed by foreign host.]

				if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP_FALLBACK))
				{
					// Can't fallback on basic authentication
					internalDisconnect();
					throw exceptions::authentication_error(response);
				}

				// Ensure connection is valid (cf. note above)
				try
				{
					string response2;
					sendRequest("NOOP");
					readResponse(response2, false);
				}
				catch (exceptions::socket_exception&)
				{
					internalDisconnect();
					throw exceptions::authentication_error(response);
				}
			}
		}
		else
		{
			// APOP not supported
			if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP_FALLBACK))
			{
				// Can't fallback on basic authentication
				internalDisconnect();
				throw exceptions::authentication_error("APOP not supported");
			}
		}
	}

	// Basic authentication
	//
	// eg:  C: USER vincent
	// ---  S: +OK vincent is a valid mailbox
	//
	//      C: PASS couic
	//      S: +OK vincent's maildrop has 2 messages (320 octets)
	sendRequest("USER " + username);
	readResponse(response, false);

	if (!isSuccessResponse(response))
	{
		internalDisconnect();
		throw exceptions::authentication_error(response);
	}

	sendRequest("PASS " + password);
	readResponse(response, false);

	if (!isSuccessResponse(response))
	{
		internalDisconnect();
		throw exceptions::authentication_error(response);
	}

	m_authentified = true;
}


#if VMIME_HAVE_SASL_SUPPORT

void POP3Store::authenticateSASL()
{
	if (!getAuthenticator().dynamicCast <security::sasl::SASLAuthenticator>())
		throw exceptions::authentication_error("No SASL authenticator available.");

	std::vector <string> capa = getCapabilities();
	std::vector <string> saslMechs;

	for (unsigned int i = 0 ; i < capa.size() ; ++i)
	{
		const string& x = capa[i];

		// C: CAPA
		// S: +OK List of capabilities follows
		// S: LOGIN-DELAY 0
		// S: PIPELINING
		// S: UIDL
		// S: ...
		// S: SASL DIGEST-MD5 CRAM-MD5   <-----
		// S: EXPIRE NEVER
		// S: ...

		if (x.length() > 5 &&
		    (x[0] == 'S' || x[0] == 's') &&
		    (x[1] == 'A' || x[1] == 'a') &&
		    (x[2] == 'S' || x[2] == 's') &&
		    (x[3] == 'L' || x[3] == 'l') &&
		    (x[4] == ' ' || x[4] == '\t'))
		{
			const string list(x.begin() + 5, x.end());

			std::istringstream iss(list);
			string mech;

			while (iss >> mech)
				saslMechs.push_back(mech);
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
			saslContext->createSession("pop3", getAuthenticator(), mech);

		saslSession->init();

		sendRequest("AUTH " + mech->getName());

		for (bool cont = true ; cont ; )
		{
			string response;
			readResponse(response, false);

			switch (getResponseCode(response))
			{
			case RESPONSE_OK:
			{
				m_socket = saslSession->getSecuredSocket(m_socket);
				return;
			}
			case RESPONSE_READY:
			{
				byte_t* challenge = 0;
				int challengeLen = 0;

				byte_t* resp = 0;
				int respLen = 0;

				try
				{
					// Extract challenge
					stripResponseCode(response, response);
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

void POP3Store::startTLS()
{
	try
	{
		sendRequest("STLS");

		string response;
		readResponse(response, false);

		if (getResponseCode(response) != RESPONSE_OK)
			throw exceptions::command_error("STLS", response);

		ref <tls::TLSSession> tlsSession =
			vmime::create <tls::TLSSession>(getCertificateVerifier());

		ref <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		tlsSocket->handshake(m_timeoutHandler);

		m_socket = tlsSocket;

		m_secured = true;
		m_cntInfos = vmime::create <tls::TLSSecuredConnectionInfos>
			(m_cntInfos->getHost(), m_cntInfos->getPort(), tlsSession, tlsSocket);
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


bool POP3Store::isConnected() const
{
	return (m_socket && m_socket->isConnected() && m_authentified);
}


bool POP3Store::isSecuredConnection() const
{
	return m_secured;
}


ref <connectionInfos> POP3Store::getConnectionInfos() const
{
	return m_cntInfos;
}


void POP3Store::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void POP3Store::internalDisconnect()
{
	for (std::list <POP3Folder*>::iterator it = m_folders.begin() ;
	     it != m_folders.end() ; ++it)
	{
		(*it)->onStoreDisconnected();
	}

	m_folders.clear();

	try
	{
		sendRequest("QUIT");
	}
	catch (exception&)
	{
		// Not important
	}

	m_socket->disconnect();
	m_socket = NULL;

	m_timeoutHandler = NULL;

	m_authentified = false;

	m_secured = false;
	m_cntInfos = NULL;
}


void POP3Store::noop()
{
	sendRequest("NOOP");

	string response;
	readResponse(response, false);

	if (!isSuccessResponse(response))
		throw exceptions::command_error("NOOP", response);
}


const std::vector <string> POP3Store::getCapabilities()
{
	sendRequest("CAPA");

	string response;
	readResponse(response, true);

	std::vector <string> res;

	if (isSuccessResponse(response))
	{
		stripFirstLine(response, response);

		std::istringstream iss(response);
		string line;

		while (std::getline(iss, line, '\n'))
			res.push_back(utility::stringUtils::trim(line));
	}

	return res;
}


bool POP3Store::isSuccessResponse(const string& buffer)
{
	return getResponseCode(buffer) == RESPONSE_OK;
}


bool POP3Store::stripFirstLine(const string& buffer, string& result, string* firstLine)
{
	const string::size_type end = buffer.find('\n');

	if (end != string::npos)
	{
		if (firstLine) *firstLine = buffer.substr(0, end);
		result = buffer.substr(end + 1);
		return (true);
	}
	else
	{
		result = buffer;
		return (false);
	}
}


int POP3Store::getResponseCode(const string& buffer)
{
	if (buffer.length() >= 2)
	{
		// +[space]
		if (buffer[0] == '+' &&
		    (buffer[1] == ' ' || buffer[1] == '\t'))
		{
			return RESPONSE_READY;
		}

		// +OK
		if (buffer.length() >= 3)
		{
			if (buffer[0] == '+' &&
			    (buffer[1] == 'O' || buffer[1] == 'o') &&
			    (buffer[2] == 'K' || buffer[1] == 'k'))
			{
				return RESPONSE_OK;
			}
		}
	}

	// -ERR or whatever
	return RESPONSE_ERR;
}


void POP3Store::stripResponseCode(const string& buffer, string& result)
{
	const string::size_type pos = buffer.find_first_of(" \t");

	if (pos != string::npos)
		result = buffer.substr(pos + 1);
	else
		result = buffer;
}


void POP3Store::sendRequest(const string& buffer, const bool end)
{
	if (end)
		m_socket->send(buffer + "\r\n");
	else
		m_socket->send(buffer);
}


void POP3Store::readResponse(string& buffer, const bool multiLine,
                             utility::progressListener* progress)
{
	bool foundTerminator = false;
	int current = 0, total = 0;

	if (progress)
		progress->start(total);

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	buffer.clear();

	string::value_type last1 = '\0', last2 = '\0';

	for ( ; !foundTerminator ; )
	{
#if 0 // not supported
		// Check for possible cancellation
		if (progress && progress->cancel())
			throw exceptions::operation_cancelled();
#endif

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
			platform::getHandler()->wait();
			continue;
		}

		// We have received data: reset the time-out counter
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		// Check for transparent characters: '\n..' becomes '\n.'
		const string::value_type first = receiveBuffer[0];

		if (first == '.' && last2 == '\n' && last1 == '.')
		{
			receiveBuffer.erase(receiveBuffer.begin());
		}
		else if (receiveBuffer.length() >= 2 && first == '.' &&
		         receiveBuffer[1] == '.' && last1 == '\n')
		{
			receiveBuffer.erase(receiveBuffer.begin());
		}

		for (string::size_type trans ;
		     string::npos != (trans = receiveBuffer.find("\n..")) ; )
		{
			receiveBuffer.replace(trans, 3, "\n.");
		}

		last1 = receiveBuffer[receiveBuffer.length() - 1];
		last2 = static_cast <char>((receiveBuffer.length() >= 2) ? receiveBuffer[receiveBuffer.length() - 2] : 0);

		// Append the data to the response buffer
		buffer += receiveBuffer;
		current += receiveBuffer.length();

		// Check for terminator string (and strip it if present)
		foundTerminator = checkTerminator(buffer, multiLine);

		// Notify progress
		if (progress)
		{
			total = std::max(total, current);
			progress->progress(current, total);
		}

		// If there is an error (-ERR) when executing a command that
		// requires a multi-line response, the error response will
		// include only one line, so we stop waiting for a multi-line
		// terminator and check for a "normal" one.
		if (multiLine && !foundTerminator && buffer.length() >= 4 && buffer[0] == '-')
		{
			foundTerminator = checkTerminator(buffer, false);
		}
	}

	if (progress)
		progress->stop(total);
}


void POP3Store::readResponse(utility::outputStream& os,
	utility::progressListener* progress, const int predictedSize)
{
	int current = 0, total = predictedSize;

	string temp;
	bool codeDone = false;

	if (progress)
		progress->start(total);

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	utility::inputStreamSocketAdapter sis(*m_socket);
	utility::stopSequenceFilteredInputStream <5> sfis1(sis, "\r\n.\r\n");
	utility::stopSequenceFilteredInputStream <3> sfis2(sfis1, "\n.\n");
	utility::dotFilteredInputStream dfis(sfis2);   // "\n.." --> "\n."

	utility::inputStream& is = dfis;

	while (!is.eof())
	{
#if 0 // not supported
		// Check for possible cancellation
		if (progress && progress->cancel())
			throw exceptions::operation_cancelled();
#endif

		// Check whether the time-out delay is elapsed
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
				throw exceptions::operation_timed_out();
		}

		// Receive data from the socket
		utility::stream::value_type buffer[65536];
		const utility::stream::size_type read = is.read(buffer, sizeof(buffer));

		if (read == 0)   // buffer is empty
		{
			platform::getHandler()->wait();
			continue;
		}

		// We have received data: reset the time-out counter
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		// If we don't have extracted the response code yet
		if (!codeDone)
		{
			temp += string(buffer, read);

			string firstLine;

			if (stripFirstLine(temp, temp, &firstLine) == true)
			{
				if (!isSuccessResponse(firstLine))
					throw exceptions::command_error("?", firstLine);

				codeDone = true;

				os.write(temp.data(), temp.length());
				temp.clear();

				continue;
			}
		}
		else
		{
			// Inject the data into the output stream
			os.write(buffer, read);
			current += read;

			// Notify progress
			if (progress)
			{
				total = std::max(total, current);
				progress->progress(current, total);
			}
		}
	}

	if (progress)
		progress->stop(total);
}


bool POP3Store::checkTerminator(string& buffer, const bool multiLine)
{
	// Multi-line response
	if (multiLine)
	{
		static const string term1("\r\n.\r\n");
		static const string term2("\n.\n");

		return (checkOneTerminator(buffer, term1) ||
		        checkOneTerminator(buffer, term2));
	}
	// Normal response
	else
	{
		static const string term1("\r\n");
		static const string term2("\n");

		return (checkOneTerminator(buffer, term1) ||
		        checkOneTerminator(buffer, term2));
	}

	return (false);
}


bool POP3Store::checkOneTerminator(string& buffer, const string& term)
{
	if (buffer.length() >= term.length() &&
		std::equal(buffer.end() - term.length(), buffer.end(), term.begin()))
	{
		buffer.erase(buffer.end() - term.length(), buffer.end());
		return (true);
	}

	return (false);
}


void POP3Store::registerFolder(POP3Folder* folder)
{
	m_folders.push_back(folder);
}


void POP3Store::unregisterFolder(POP3Folder* folder)
{
	std::list <POP3Folder*>::iterator it = std::find(m_folders.begin(), m_folders.end(), folder);
	if (it != m_folders.end()) m_folders.erase(it);
}


int POP3Store::getCapabilities() const
{
	return (CAPABILITY_DELETE_MESSAGE);
}



// Service infos

POP3ServiceInfos POP3Store::sm_infos(false);


const serviceInfos& POP3Store::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& POP3Store::getInfos() const
{
	return sm_infos;
}


} // pop3
} // net
} // vmime

