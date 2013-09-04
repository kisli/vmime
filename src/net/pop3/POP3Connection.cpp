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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3Connection.hpp"
#include "vmime/net/pop3/POP3Store.hpp"

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
	(m_store.acquire()->getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const POP3ServiceInfos&>(m_store.acquire()->getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(m_store.acquire()->getInfos().hasProperty(getSession(), \
		dynamic_cast <const POP3ServiceInfos&>(m_store.acquire()->getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace pop3 {



POP3Connection::POP3Connection(ref <POP3Store> store, ref <security::authenticator> auth)
	: m_store(store), m_auth(auth), m_socket(NULL), m_timeoutHandler(NULL),
	  m_authenticated(false), m_secured(false), m_capabilitiesFetched(false)
{
}


POP3Connection::~POP3Connection()
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


void POP3Connection::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	ref <POP3Store> store = m_store.acquire();

	// Create the time-out handler
	if (store->getTimeoutHandlerFactory())
		m_timeoutHandler = store->getTimeoutHandlerFactory()->create();

	// Create and connect the socket
	m_socket = store->getSocketFactory()->create(m_timeoutHandler);

#if VMIME_HAVE_TLS_SUPPORT
	if (store->isPOP3S())  // dedicated port/POP3S
	{
		ref <tls::TLSSession> tlsSession =
			tls::TLSSession::create(store->getCertificateVerifier());

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

	ref <POP3Response> response = POP3Response::readResponse
		(thisRef().dynamicCast <POP3Connection>());

	if (!response->isSuccess())
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(response->getFirstLine());
	}

#if VMIME_HAVE_TLS_SUPPORT
	// Setup secured connection, if requested
	const bool tls = HAS_PROPERTY(PROPERTY_CONNECTION_TLS)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS);
	const bool tlsRequired = HAS_PROPERTY(PROPERTY_CONNECTION_TLS_REQUIRED)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS_REQUIRED);

	if (!store->isPOP3S() && tls)  // only if not POP3S
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
	authenticate(messageId(response->getText()));
}


void POP3Connection::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void POP3Connection::internalDisconnect()
{
	if (m_socket)
	{
		if (m_socket->isConnected())
		{
			try
			{
				POP3Command::QUIT()->send(thisRef().dynamicCast <POP3Connection>());
				POP3Response::readResponse(thisRef().dynamicCast <POP3Connection>());
			}
			catch (exception&)
			{
				// Not important
			}

			m_socket->disconnect();
		}

		m_socket = NULL;
	}

	m_timeoutHandler = NULL;

	m_authenticated = false;
	m_secured = false;

	m_cntInfos = NULL;
}


void POP3Connection::authenticate(const messageId& randomMID)
{
	getAuthenticator()->setService(m_store.acquire());

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

	ref <POP3Connection> conn = thisRef().dynamicCast <POP3Connection>();
	ref <POP3Response> response;

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

			POP3Command::APOP(username, md5->getHexDigest())->send(conn);
			response = POP3Response::readResponse(conn);

			if (response->isSuccess())
			{
				m_authenticated = true;
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
					throw exceptions::authentication_error(response->getFirstLine());
				}

				// Ensure connection is valid (cf. note above)
				try
				{
					POP3Command::NOOP()->send(conn);
					POP3Response::readResponse(conn);
				}
				catch (exceptions::socket_exception&)
				{
					internalDisconnect();
					throw exceptions::authentication_error(response->getFirstLine());
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
	POP3Command::USER(username)->send(conn);
	response = POP3Response::readResponse(conn);

	if (!response->isSuccess())
	{
		internalDisconnect();
		throw exceptions::authentication_error(response->getFirstLine());
	}

	POP3Command::PASS(password)->send(conn);
	response = POP3Response::readResponse(conn);

	if (!response->isSuccess())
	{
		internalDisconnect();
		throw exceptions::authentication_error(response->getFirstLine());
	}

	m_authenticated = true;
}


#if VMIME_HAVE_SASL_SUPPORT

void POP3Connection::authenticateSASL()
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

		POP3Command::AUTH(mech->getName())->send(thisRef().dynamicCast <POP3Connection>());

		for (bool cont = true ; cont ; )
		{
			ref <POP3Response> response =
				POP3Response::readResponse(thisRef().dynamicCast <POP3Connection>());

			switch (response->getCode())
			{
			case POP3Response::CODE_OK:
			{
				m_socket = saslSession->getSecuredSocket(m_socket);
				return;
			}
			case POP3Response::CODE_READY:
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

void POP3Connection::startTLS()
{
	try
	{
		POP3Command::STLS()->send(thisRef().dynamicCast <POP3Connection>());

		ref <POP3Response> response =
			POP3Response::readResponse(thisRef().dynamicCast <POP3Connection>());

		if (!response->isSuccess())
			throw exceptions::command_error("STLS", response->getFirstLine());

		ref <tls::TLSSession> tlsSession =
			tls::TLSSession::create(m_store.acquire()->getCertificateVerifier());

		ref <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		tlsSocket->handshake(m_timeoutHandler);

		m_socket = tlsSocket;

		m_secured = true;
		m_cntInfos = vmime::create <tls::TLSSecuredConnectionInfos>
			(m_cntInfos->getHost(), m_cntInfos->getPort(), tlsSession, tlsSocket);

		// " Once TLS has been started, the client MUST discard cached
		//   information about server capabilities and SHOULD re-issue
		//   the CAPA command.  This is necessary to protect against
		//   man-in-the-middle attacks which alter the capabilities list
		//   prior to STLS. " (RFC-2595)
		invalidateCapabilities();
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


const std::vector <string> POP3Connection::getCapabilities()
{
	if (!m_capabilitiesFetched)
		fetchCapabilities();

	return m_capabilities;
}


void POP3Connection::invalidateCapabilities()
{
	m_capabilities.clear();
	m_capabilitiesFetched = false;
}


void POP3Connection::fetchCapabilities()
{
	POP3Command::CAPA()->send(thisRef().dynamicCast <POP3Connection>());

	ref <POP3Response> response =
		POP3Response::readMultilineResponse(thisRef().dynamicCast <POP3Connection>());

	std::vector <string> res;

	if (response->isSuccess())
	{
		for (size_t i = 0, n = response->getLineCount() ; i < n ; ++i)
			res.push_back(response->getLineAt(i));
	}

	m_capabilities = res;
	m_capabilitiesFetched = true;
}


bool POP3Connection::isConnected() const
{
	return m_socket && m_socket->isConnected() && m_authenticated;
}


bool POP3Connection::isSecuredConnection() const
{
	return m_secured;
}


ref <connectionInfos> POP3Connection::getConnectionInfos() const
{
	return m_cntInfos;
}


ref <POP3Store> POP3Connection::getStore()
{
	return m_store.acquire();
}


ref <session> POP3Connection::getSession()
{
	return m_store.acquire()->getSession();
}


ref <socket> POP3Connection::getSocket()
{
	return m_socket;
}


ref <timeoutHandler> POP3Connection::getTimeoutHandler()
{
	return m_timeoutHandler;
}


ref <security::authenticator> POP3Connection::getAuthenticator()
{
	return m_auth;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3
