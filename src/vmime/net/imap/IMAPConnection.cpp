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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPTag.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPCommand.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/net/defaultConnectionInfos.hpp"

#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/SASLContext.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/net/tls/TLSSession.hpp"
	#include "vmime/net/tls/TLSSecuredConnectionInfos.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT

#include <sstream>


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(m_store.lock()->getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const IMAPServiceInfos&>(m_store.lock()->getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(m_store.lock()->getInfos().hasProperty(getSession(), \
		dynamic_cast <const IMAPServiceInfos&>(m_store.lock()->getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace imap {


IMAPConnection::IMAPConnection(shared_ptr <IMAPStore> store, shared_ptr <security::authenticator> auth)
	: m_store(store), m_auth(auth), m_socket(null), m_parser(null), m_tag(null),
	  m_hierarchySeparator('\0'), m_state(STATE_NONE), m_timeoutHandler(null),
	  m_secured(false), m_firstTag(true), m_capabilitiesFetched(false), m_noModSeq(false)
{
	static int connectionId = 0;

	m_tag = make_shared <IMAPTag>();

	if (store->getTracerFactory())
		m_tracer = store->getTracerFactory()->create(store, ++connectionId);

	m_parser = make_shared <IMAPParser>();
	m_parser->setTag(m_tag);
	m_parser->setTracer(m_tracer);
}


IMAPConnection::~IMAPConnection()
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


void IMAPConnection::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_state = STATE_NONE;
	m_hierarchySeparator = '\0';

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	shared_ptr <IMAPStore> store = m_store.lock();

	// Create the time-out handler
	if (store->getTimeoutHandlerFactory())
		m_timeoutHandler = store->getTimeoutHandlerFactory()->create();

	// Create and connect the socket
	m_socket = store->getSocketFactory()->create(m_timeoutHandler);
	m_socket->setTracer(m_tracer);

#if VMIME_HAVE_TLS_SUPPORT
	if (store->isIMAPS())  // dedicated port/IMAPS
	{
		shared_ptr <tls::TLSSession> tlsSession = tls::TLSSession::create
			(store->getCertificateVerifier(),
			 store->getSession()->getTLSProperties());

		shared_ptr <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		m_socket = tlsSocket;

		m_secured = true;
		m_cntInfos = make_shared <tls::TLSSecuredConnectionInfos>(address, port, tlsSession, tlsSocket);
	}
	else
#endif // VMIME_HAVE_TLS_SUPPORT
	{
		m_cntInfos = make_shared <defaultConnectionInfos>(address, port);
	}

	m_socket->connect(address, port);


	m_parser->setSocket(m_socket);
	m_parser->setTimeoutHandler(m_timeoutHandler);


	setState(STATE_NON_AUTHENTICATED);


	// Connection greeting
	//
	// eg:  C: <connection to server>
	// ---  S: * OK mydomain.org IMAP4rev1 v12.256 server ready

	std::auto_ptr <IMAPParser::greeting> greet(m_parser->readGreeting());
	bool needAuth = false;

	if (greet->resp_cond_bye())
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(greet->getErrorLog());
	}
	else if (greet->resp_cond_auth()->condition() != IMAPParser::resp_cond_auth::PREAUTH)
	{
		needAuth = true;
	}

	if (greet->resp_cond_auth()->resp_text()->resp_text_code() &&
	    greet->resp_cond_auth()->resp_text()->resp_text_code()->capability_data())
	{
		processCapabilityResponseData(greet->resp_cond_auth()->resp_text()->resp_text_code()->capability_data());
	}

#if VMIME_HAVE_TLS_SUPPORT
	// Setup secured connection, if requested
	const bool tls = HAS_PROPERTY(PROPERTY_CONNECTION_TLS)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS);
	const bool tlsRequired = HAS_PROPERTY(PROPERTY_CONNECTION_TLS_REQUIRED)
		&& GET_PROPERTY(bool, PROPERTY_CONNECTION_TLS_REQUIRED);

	if (!store->isIMAPS() && tls)  // only if not IMAPS
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
				m_state = STATE_NONE;
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
			m_state = STATE_NONE;
			throw;
		}
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	// Authentication
	if (needAuth)
	{
		try
		{
			authenticate();
		}
		catch (...)
		{
			m_state = STATE_NONE;
			throw;
		}
	}

	// Get the hierarchy separator character
	initHierarchySeparator();

	// Switch to state "Authenticated"
	setState(STATE_AUTHENTICATED);
}


void IMAPConnection::authenticate()
{
	getAuthenticator()->setService(m_store.lock());

#if VMIME_HAVE_SASL_SUPPORT
	// First, try SASL authentication
	if (GET_PROPERTY(bool, PROPERTY_OPTIONS_SASL))
	{
		try
		{
			authenticateSASL();
			return;
		}
		catch (exceptions::authentication_error&)
		{
			if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_SASL_FALLBACK))
			{
				// Can't fallback on normal authentication
				internalDisconnect();
				throw;
			}
			else
			{
				// Ignore, will try normal authentication
			}
		}
		catch (exception&)
		{
			internalDisconnect();
			throw;
		}
	}
#endif // VMIME_HAVE_SASL_SUPPORT

	// Normal authentication
	const string username = getAuthenticator()->getUsername();
	const string password = getAuthenticator()->getPassword();

	shared_ptr <IMAPConnection> conn = dynamicCast <IMAPConnection>(shared_from_this());
	IMAPCommand::LOGIN(username, password)->send(conn);

	std::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

	if (resp->isBad())
	{
		internalDisconnect();
		throw exceptions::command_error("LOGIN", resp->getErrorLog());
	}
	else if (resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		internalDisconnect();
		throw exceptions::authentication_error(resp->getErrorLog());
	}

	// Server capabilities may change when logged in
	if (!processCapabilityResponseData(resp.get()))
		invalidateCapabilities();
}


#if VMIME_HAVE_SASL_SUPPORT

void IMAPConnection::authenticateSASL()
{
	if (!dynamicCast <security::sasl::SASLAuthenticator>(getAuthenticator()))
		throw exceptions::authentication_error("No SASL authenticator available.");

	const std::vector <string> capa = getCapabilities();
	std::vector <string> saslMechs;

	for (unsigned int i = 0 ; i < capa.size() ; ++i)
	{
		const string& x = capa[i];

		if (x.length() > 5 &&
		    (x[0] == 'A' || x[0] == 'a') &&
		    (x[1] == 'U' || x[1] == 'u') &&
		    (x[2] == 'T' || x[2] == 't') &&
		    (x[3] == 'H' || x[3] == 'h') &&
		    x[4] == '=')
		{
			saslMechs.push_back(string(x.begin() + 5, x.end()));
		}
	}

	if (saslMechs.empty())
		throw exceptions::authentication_error("No SASL mechanism available.");

	std::vector <shared_ptr <security::sasl::SASLMechanism> > mechList;

	shared_ptr <security::sasl::SASLContext> saslContext =
		make_shared <security::sasl::SASLContext>();

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
	shared_ptr <security::sasl::SASLMechanism> suggestedMech =
		saslContext->suggestMechanism(mechList);

	if (!suggestedMech)
		throw exceptions::authentication_error("Unable to suggest SASL mechanism.");

	// Allow application to choose which mechanisms to use
	mechList = dynamicCast <security::sasl::SASLAuthenticator>(getAuthenticator())->
		getAcceptableMechanisms(mechList, suggestedMech);

	if (mechList.empty())
		throw exceptions::authentication_error("No SASL mechanism available.");

	// Try each mechanism in the list in turn
	for (unsigned int i = 0 ; i < mechList.size() ; ++i)
	{
		shared_ptr <security::sasl::SASLMechanism> mech = mechList[i];

		shared_ptr <security::sasl::SASLSession> saslSession =
			saslContext->createSession("imap", getAuthenticator(), mech);

		saslSession->init();

		shared_ptr <IMAPCommand> authCmd;

		if (saslSession->getMechanism()->hasInitialResponse())
		{
			byte_t* initialResp = 0;
			size_t initialRespLen = 0;

			saslSession->evaluateChallenge(NULL, 0, &initialResp, &initialRespLen);

			string encodedInitialResp(saslContext->encodeB64(initialResp, initialRespLen));
			delete [] initialResp;

			if (encodedInitialResp.empty())
				authCmd = IMAPCommand::AUTHENTICATE(mech->getName(), "=");
			else
				authCmd = IMAPCommand::AUTHENTICATE(mech->getName(), encodedInitialResp);
		}
		else
		{
			authCmd = IMAPCommand::AUTHENTICATE(mech->getName());
		}

		authCmd->send(dynamicCast <IMAPConnection>(shared_from_this()));

		for (bool cont = true ; cont ; )
		{
			std::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

			if (resp->response_done() &&
			    resp->response_done()->response_tagged() &&
			    resp->response_done()->response_tagged()->resp_cond_state()->
			    	status() == IMAPParser::resp_cond_state::OK)
			{
				m_socket = saslSession->getSecuredSocket(m_socket);
				return;
			}
			else
			{
				std::vector <IMAPParser::continue_req_or_response_data*>
					respDataList = resp->continue_req_or_response_data();

				string response;
				bool hasResponse = false;

				for (unsigned int i = 0 ; i < respDataList.size() ; ++i)
				{
					if (respDataList[i]->continue_req())
					{
						response = respDataList[i]->continue_req()->resp_text()->text();
						hasResponse = true;
						break;
					}
				}

				if (!hasResponse)
				{
					cont = false;
					continue;
				}

				byte_t* challenge = 0;
				size_t challengeLen = 0;

				byte_t* resp = 0;
				size_t respLen = 0;

				try
				{
					// Extract challenge
					saslContext->decodeB64(response, &challenge, &challengeLen);

					// Prepare response
					saslSession->evaluateChallenge
						(challenge, challengeLen, &resp, &respLen);

					// Send response
					const string respB64 = saslContext->encodeB64(resp, respLen) + "\r\n";
					sendRaw(utility::stringUtils::bytesFromString(respB64), respB64.length());

					if (m_tracer)
						m_tracer->traceSendBytes(respB64.length() - 2, "SASL exchange");

					// Server capabilities may change when logged in
					invalidateCapabilities();
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
					sendRaw(utility::stringUtils::bytesFromString("*\r\n"), 3);

					if (m_tracer)
						m_tracer->traceSend("*");
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
			}
		}
	}

	throw exceptions::authentication_error
		("Could not authenticate using SASL: all mechanisms failed.");
}

#endif // VMIME_HAVE_SASL_SUPPORT


#if VMIME_HAVE_TLS_SUPPORT

void IMAPConnection::startTLS()
{
	try
	{
		IMAPCommand::STARTTLS()->send(dynamicCast <IMAPConnection>(shared_from_this()));

		std::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

		if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			throw exceptions::command_error
				("STARTTLS", resp->getErrorLog(), "bad response");
		}

		shared_ptr <tls::TLSSession> tlsSession = tls::TLSSession::create
			(m_store.lock()->getCertificateVerifier(),
			 m_store.lock()->getSession()->getTLSProperties());

		shared_ptr <tls::TLSSocket> tlsSocket =
			tlsSession->getSocket(m_socket);

		tlsSocket->handshake();

		m_socket = tlsSocket;
		m_parser->setSocket(m_socket);

		m_secured = true;
		m_cntInfos = make_shared <tls::TLSSecuredConnectionInfos>
			(m_cntInfos->getHost(), m_cntInfos->getPort(), tlsSession, tlsSocket);

		// " Once TLS has been started, the client MUST discard cached
		//   information about server capabilities and SHOULD re-issue the
		//   CAPABILITY command.  This is necessary to protect against
		//   man-in-the-middle attacks which alter the capabilities list prior
		//   to STARTTLS. " (RFC-2595)
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


const std::vector <string> IMAPConnection::getCapabilities()
{
	if (!m_capabilitiesFetched)
		fetchCapabilities();

	return m_capabilities;
}


bool IMAPConnection::hasCapability(const string& capa)
{
	if (!m_capabilitiesFetched)
		fetchCapabilities();

	const string normCapa = utility::stringUtils::toUpper(capa);

	for (size_t i = 0, n = m_capabilities.size() ; i < n ; ++i)
	{
		if (m_capabilities[i] == normCapa)
			return true;
	}

	return false;
}


bool IMAPConnection::hasCapability(const string& capa) const
{
	const string normCapa = utility::stringUtils::toUpper(capa);

	for (size_t i = 0, n = m_capabilities.size() ; i < n ; ++i)
	{
		if (m_capabilities[i] == normCapa)
			return true;
	}

	return false;
}


void IMAPConnection::invalidateCapabilities()
{
	m_capabilities.clear();
	m_capabilitiesFetched = false;
}


void IMAPConnection::fetchCapabilities()
{
	IMAPCommand::CAPABILITY()->send(dynamicCast <IMAPConnection>(shared_from_this()));

	std::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

	if (resp->response_done()->response_tagged()->
		resp_cond_state()->status() == IMAPParser::resp_cond_state::OK)
	{
		processCapabilityResponseData(resp.get());
	}
}


bool IMAPConnection::processCapabilityResponseData(const IMAPParser::response* resp)
{
	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	for (size_t i = 0 ; i < respDataList.size() ; ++i)
	{
		if (respDataList[i]->response_data() == NULL)
			continue;

		const IMAPParser::capability_data* capaData =
			respDataList[i]->response_data()->capability_data();

		if (capaData == NULL)
			continue;

		processCapabilityResponseData(capaData);
		return true;
	}

	return false;
}


void IMAPConnection::processCapabilityResponseData(const IMAPParser::capability_data* capaData)
{
	std::vector <string> res;

	std::vector <IMAPParser::capability*> caps = capaData->capabilities();

	for (unsigned int j = 0 ; j < caps.size() ; ++j)
	{
		if (caps[j]->auth_type())
			res.push_back("AUTH=" + caps[j]->auth_type()->name());
		else
			res.push_back(utility::stringUtils::toUpper(caps[j]->atom()->value()));
	}

	m_capabilities = res;
	m_capabilitiesFetched = true;
}


shared_ptr <security::authenticator> IMAPConnection::getAuthenticator()
{
	return m_auth;
}


bool IMAPConnection::isConnected() const
{
	return (m_socket && m_socket->isConnected() &&
	        (m_state == STATE_AUTHENTICATED || m_state == STATE_SELECTED));
}


bool IMAPConnection::isSecuredConnection() const
{
	return m_secured;
}


shared_ptr <connectionInfos> IMAPConnection::getConnectionInfos() const
{
	return m_cntInfos;
}


void IMAPConnection::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void IMAPConnection::internalDisconnect()
{
	if (isConnected())
	{
		IMAPCommand::LOGOUT()->send(dynamicCast <IMAPConnection>(shared_from_this()));

		m_socket->disconnect();
		m_socket = null;
	}

	m_timeoutHandler = null;

	m_state = STATE_LOGOUT;

	m_secured = false;
	m_cntInfos = null;
}


void IMAPConnection::initHierarchySeparator()
{
	IMAPCommand::LIST("", "")->send(dynamicCast <IMAPConnection>(shared_from_this()));

	std::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		internalDisconnect();
		throw exceptions::command_error("LIST", resp->getErrorLog(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	bool found = false;

	for (unsigned int i = 0 ; !found && i < respDataList.size() ; ++i)
	{
		if (respDataList[i]->response_data() == NULL)
			continue;

		const IMAPParser::mailbox_data* mailboxData =
			static_cast <const IMAPParser::response_data*>
				(respDataList[i]->response_data())->mailbox_data();

		if (mailboxData == NULL || mailboxData->type() != IMAPParser::mailbox_data::LIST)
			continue;

		if (mailboxData->mailbox_list()->quoted_char() != '\0')
		{
			m_hierarchySeparator = mailboxData->mailbox_list()->quoted_char();
			found = true;
		}
	}

	if (!found) // default
		m_hierarchySeparator = '/';
}


void IMAPConnection::sendCommand(shared_ptr <IMAPCommand> cmd)
{
	if (!m_firstTag)
		++(*m_tag);

	m_socket->send(*m_tag);
	m_socket->send(" ");
	m_socket->send(cmd->getText());
	m_socket->send("\r\n");

	m_firstTag = false;

	if (m_tracer)
	{
		std::ostringstream oss;
		oss << string(*m_tag) << " " << cmd->getText();

		m_tracer->traceSend(oss.str());
	}
}


void IMAPConnection::sendRaw(const byte_t* buffer, const size_t count)
{
	m_socket->sendRaw(buffer, count);
}


IMAPParser::response* IMAPConnection::readResponse(IMAPParser::literalHandler* lh)
{
	return (m_parser->readResponse(lh));
}


IMAPConnection::ProtocolStates IMAPConnection::state() const
{
	return (m_state);
}


void IMAPConnection::setState(const ProtocolStates state)
{
	m_state = state;
}


char IMAPConnection::hierarchySeparator() const
{
	return (m_hierarchySeparator);
}


shared_ptr <const IMAPStore> IMAPConnection::getStore() const
{
	return m_store.lock();
}


shared_ptr <IMAPStore> IMAPConnection::getStore()
{
	return m_store.lock();
}


shared_ptr <session> IMAPConnection::getSession()
{
	return m_store.lock()->getSession();
}


shared_ptr <const socket> IMAPConnection::getSocket() const
{
	return m_socket;
}


void IMAPConnection::setSocket(shared_ptr <socket> sok)
{
	m_socket = sok;
	m_parser->setSocket(sok);
}


shared_ptr <tracer> IMAPConnection::getTracer()
{
	return m_tracer;
}


shared_ptr <IMAPTag> IMAPConnection::getTag()
{
	return m_tag;
}


bool IMAPConnection::isMODSEQDisabled() const
{
	return m_noModSeq;
}


void IMAPConnection::disableMODSEQ()
{
	m_noModSeq = true;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

