//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPConnection.hpp"
#include "vmime/net/smtp/SMTPTransport.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/security/digest/messageDigestFactory.hpp"

#include "vmime/net/defaultConnectionInfos.hpp"

#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/SASLContext.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/net/tls/TLSSession.hpp"
	#include "vmime/net/tls/TLSSecuredConnectionInfos.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT



// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(m_transport.acquire()->getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const SMTPServiceInfos&>(m_transport.acquire()->getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(m_transport.acquire()->getInfos().hasProperty(getSession(), \
		dynamic_cast <const SMTPServiceInfos&>(m_transport.acquire()->getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace smtp {



SMTPConnection::SMTPConnection(ref <SMTPTransport> transport, ref <security::authenticator> auth)
	: m_transport(transport), m_auth(auth), m_socket(NULL), m_timeoutHandler(NULL),
	  m_authenticated(false), m_secured(false), m_extendedSMTP(false)
{
}


SMTPConnection::~SMTPConnection()
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


void SMTPConnection::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	ref <SMTPTransport> transport = m_transport.acquire();

	// Create the time-out handler
	if (transport->getTimeoutHandlerFactory())
		m_timeoutHandler = transport->getTimeoutHandlerFactory()->create();

	// Create and connect the socket
	m_socket = transport->getSocketFactory()->create(m_timeoutHandler);

#if VMIME_HAVE_TLS_SUPPORT
	if (transport->isSMTPS())  // dedicated port/SMTPS
	{
		ref <tls::TLSSession> tlsSession =
			tls::TLSSession::create(transport->getCertificateVerifier());

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
	// ---  S: 220 smtp.domain.com Service ready

	ref <SMTPResponse> resp;

	if ((resp = readResponse())->getCode() != 220)
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(resp->getText());
	}

	// Identification
	helo();

#if VMIME_HAVE_TLS_SUPPORT
	// Setup secured connection, if requested
	const bool tls = HAS_PROPERTY(PROPERTY_CONNECTION_TLS)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS);
	const bool tlsRequired = HAS_PROPERTY(PROPERTY_CONNECTION_TLS_REQUIRED)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS_REQUIRED);

	if (!transport->isSMTPS() && tls)  // only if not SMTPS
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

		// Must reissue a EHLO command [RFC-2487, 5.2]
		helo();
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	// Authentication
	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_NEEDAUTH))
		authenticate();
	else
		m_authenticated = true;
}


void SMTPConnection::helo()
{
	// First, try Extended SMTP (ESMTP)
	//
	// eg:  C: EHLO thismachine.ourdomain.com
	//      S: 250-smtp.theserver.com
	//      S: 250-AUTH CRAM-MD5 DIGEST-MD5
	//      S: 250-PIPELINING
	//      S: 250 SIZE 2555555555

	sendRequest(SMTPCommand::EHLO(platform::getHandler()->getHostName()));

	ref <SMTPResponse> resp;

	if ((resp = readResponse())->getCode() != 250)
	{
		// Next, try "Basic" SMTP
		//
		// eg:  C: HELO thismachine.ourdomain.com
		//      S: 250 OK

		sendRequest(SMTPCommand::HELO(platform::getHandler()->getHostName()));

		if ((resp = readResponse())->getCode() != 250)
		{
			internalDisconnect();
			throw exceptions::connection_greeting_error(resp->getLastLine().getText());
		}

		m_extendedSMTP = false;
		m_extensions.clear();
	}
	else
	{
		m_extendedSMTP = true;
		m_extensions.clear();

		// Get supported extensions from SMTP response
		// One extension per line, format is: EXT PARAM1 PARAM2...
		for (size_t i = 1, n = resp->getLineCount() ; i < n ; ++i)
		{
			const string line = resp->getLineAt(i).getText();
			std::istringstream iss(line);

			string ext;
			iss >> ext;

			std::vector <string> params;
			string param;

			// Special case: some servers send "AUTH=MECH [MECH MECH...]"
			if (ext.length() >= 5 && utility::stringUtils::toUpper(ext.substr(0, 5)) == "AUTH=")
			{
				params.push_back(utility::stringUtils::toUpper(ext.substr(5)));
				ext = "AUTH";
			}

			while (iss >> param)
				params.push_back(utility::stringUtils::toUpper(param));

			m_extensions[ext] = params;
		}
	}
}


bool SMTPConnection::hasExtension
	(const std::string& extName, std::vector <string>* params) const
{
	std::map <string, std::vector <string> >::const_iterator
		it = m_extensions.find(extName);

	if (it != m_extensions.end())
	{
		if (params)
			*params = (*it).second;

		return true;
	}
	else
	{
		return false;
	}
}


void SMTPConnection::authenticate()
{
	if (!m_extendedSMTP)
	{
		internalDisconnect();
		throw exceptions::command_error("AUTH", "ESMTP not supported.");
	}

	getAuthenticator()->setService(m_transport.acquire());

#if VMIME_HAVE_SASL_SUPPORT
	// First, try SASL authentication
	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_SASL))
	{
		try
		{
			authenticateSASL();

			m_authenticated = true;
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

void SMTPConnection::authenticateSASL()
{
	if (!getAuthenticator().dynamicCast <security::sasl::SASLAuthenticator>())
		throw exceptions::authentication_error("No SASL authenticator available.");

	// Obtain SASL mechanisms supported by server from ESMTP extensions
	std::vector <string> saslMechs;
	hasExtension("AUTH", &saslMechs);

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

		sendRequest(SMTPCommand::AUTH(mech->getName()));

		for (bool cont = true ; cont ; )
		{
			ref <SMTPResponse> response = readResponse();

			switch (response->getCode())
			{
			case 235:
			{
				m_socket = saslSession->getSecuredSocket(m_socket);
				return;
			}
			case 334:
			{
				byte_t* challenge = 0;
				long challengeLen = 0;

				byte_t* resp = 0;
				long respLen = 0;

				try
				{
					// Extract challenge
					saslContext->decodeB64(response->getText(), &challenge, &challengeLen);

					// Prepare response
					saslSession->evaluateChallenge
						(challenge, challengeLen, &resp, &respLen);

					// Send response
					m_socket->send(saslContext->encodeB64(resp, respLen) + "\r\n");
				}
				catch (exceptions::sasl_exception& /*e*/)
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
					m_socket->sendRaw("*\r\n", 3);
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

void SMTPConnection::startTLS()
{
	try
	{
		sendRequest(SMTPCommand::STARTTLS());

		ref <SMTPResponse> resp = readResponse();

		if (resp->getCode() != 220)
			throw exceptions::command_error("STARTTLS", resp->getText());

		ref <tls::TLSSession> tlsSession =
			tls::TLSSession::create(getTransport()->getCertificateVerifier());

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


void SMTPConnection::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void SMTPConnection::internalDisconnect()
{
	try
	{
		sendRequest(SMTPCommand::QUIT());
		readResponse();
	}
	catch (exception&)
	{
		// Not important
	}

	m_socket->disconnect();
	m_socket = NULL;

	m_timeoutHandler = NULL;

	m_authenticated = false;
	m_extendedSMTP = false;

	m_secured = false;
	m_cntInfos = NULL;
}


void SMTPConnection::sendRequest(ref <SMTPCommand> cmd)
{
	cmd->writeToSocket(m_socket);
}


ref <SMTPResponse> SMTPConnection::readResponse()
{
	ref <SMTPResponse> resp = SMTPResponse::readResponse
		(m_socket, m_timeoutHandler, m_responseState);

	m_responseState = resp->getCurrentState();

	return resp;
}


bool SMTPConnection::isConnected() const
{
	return m_socket && m_socket->isConnected() && m_authenticated;
}


bool SMTPConnection::isSecuredConnection() const
{
	return m_secured;
}


ref <connectionInfos> SMTPConnection::getConnectionInfos() const
{
	return m_cntInfos;
}


ref <SMTPTransport> SMTPConnection::getTransport()
{
	return m_transport.acquire();
}


ref <session> SMTPConnection::getSession()
{
	return m_transport.acquire()->getSession();
}


ref <socket> SMTPConnection::getSocket()
{
	return m_socket;
}


ref <timeoutHandler> SMTPConnection::getTimeoutHandler()
{
	return m_timeoutHandler;
}


ref <security::authenticator> SMTPConnection::getAuthenticator()
{
	return m_auth;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
