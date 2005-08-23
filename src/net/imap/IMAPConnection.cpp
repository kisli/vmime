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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/net/imap/IMAPTag.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPStore.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include <sstream>


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(m_store->getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const IMAPStore::_infos&>(m_store->getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(m_store->getInfos().hasProperty(getSession(), \
		dynamic_cast <const IMAPStore::_infos&>(m_store->getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace imap {


IMAPConnection::IMAPConnection(weak_ref <IMAPStore> store, ref <authenticator> auth)
	: m_store(store), m_auth(auth), m_socket(NULL), m_parser(NULL), m_tag(NULL),
	  m_hierarchySeparator('\0'), m_state(STATE_NONE), m_timeoutHandler(NULL)
{
}


IMAPConnection::~IMAPConnection()
{
	if (isConnected())
		disconnect();
	else if (m_socket)
		internalDisconnect();
}


void IMAPConnection::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_state = STATE_NONE;
	m_hierarchySeparator = '\0';

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
	socketFactory* sf = platformDependant::getHandler()->
		getSocketFactory(GET_PROPERTY(string, PROPERTY_SERVER_SOCKETFACTORY));

	m_socket = sf->create();
	m_socket->connect(address, port);


	m_tag = vmime::create <IMAPTag>();
	m_parser = vmime::create <IMAPParser>(m_tag, m_socket, m_timeoutHandler);


	setState(STATE_NON_AUTHENTICATED);


	// Connection greeting
	//
	// eg:  C: <connection to server>
	// ---  S: * OK mydomain.org IMAP4rev1 v12.256 server ready

	utility::auto_ptr <IMAPParser::greeting> greet(m_parser->readGreeting());

	if (greet->resp_cond_bye())
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(m_parser->lastLine());
	}
	else if (greet->resp_cond_auth()->condition() != IMAPParser::resp_cond_auth::PREAUTH)
	{
		const authenticationInfos auth = m_auth->requestAuthInfos();

		// TODO: other authentication methods

		send(true, "LOGIN " + IMAPUtils::quoteString(auth.getUsername())
			+ " " + IMAPUtils::quoteString(auth.getPassword()), true);

		utility::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

		if (resp->isBad())
		{
			internalDisconnect();
			throw exceptions::command_error("LOGIN", m_parser->lastLine());
		}
		else if (resp->response_done()->response_tagged()->
				resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			internalDisconnect();
			throw exceptions::authentication_error(m_parser->lastLine());
		}
	}

	// Get the hierarchy separator character
	initHierarchySeparator();

	// Switch to state "Authenticated"
	setState(STATE_AUTHENTICATED);
}


const bool IMAPConnection::isConnected() const
{
	return (m_socket && m_socket->isConnected() &&
	        (m_state == STATE_AUTHENTICATED || m_state == STATE_SELECTED));
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
		send(true, "LOGOUT", true);

		m_socket->disconnect();
		m_socket = NULL;
	}

	m_timeoutHandler = NULL;

	m_state = STATE_LOGOUT;
}


void IMAPConnection::initHierarchySeparator()
{
	send(true, "LIST \"\" \"\"", true);

	vmime::utility::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		internalDisconnect();
		throw exceptions::command_error("LIST", m_parser->lastLine(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	if (respDataList.size() < 1 || respDataList[0]->response_data() == NULL)
	{
		internalDisconnect();
		throw exceptions::command_error("LIST", m_parser->lastLine(), "unexpected response");
	}

	const IMAPParser::mailbox_data* mailboxData =
		static_cast <const IMAPParser::response_data*>(respDataList[0]->response_data())->
			mailbox_data();

	if (mailboxData == NULL || mailboxData->type() != IMAPParser::mailbox_data::LIST)
	{
		internalDisconnect();
		throw exceptions::command_error("LIST", m_parser->lastLine(), "invalid type");
	}

	if (mailboxData->mailbox_list()->quoted_char() == '\0')
	{
		internalDisconnect();
		throw exceptions::command_error("LIST", m_parser->lastLine(), "no hierarchy separator");
	}

	m_hierarchySeparator = mailboxData->mailbox_list()->quoted_char();
}


void IMAPConnection::send(bool tag, const string& what, bool end)
{
#if VMIME_DEBUG
	std::ostringstream oss;

	if (tag)
	{
		++(*m_tag);

		oss << string(*m_tag);
		oss << " ";
	}

	oss << what;

	if (end)
		oss << "\r\n";

	m_socket->send(oss.str());
#else
	if (tag)
	{
		++(*m_tag);

		m_socket->send(*m_tag);
		m_socket->send(" ");
	}

	m_socket->send(what);

	if (end)
	{
		m_socket->send("\r\n");
	}
#endif
}


void IMAPConnection::sendRaw(const char* buffer, const int count)
{
	m_socket->sendRaw(buffer, count);
}


IMAPParser::response* IMAPConnection::readResponse(IMAPParser::literalHandler* lh)
{
	return (m_parser->readResponse(lh));
}


const IMAPConnection::ProtocolStates IMAPConnection::state() const
{
	return (m_state);
}


void IMAPConnection::setState(const ProtocolStates state)
{
	m_state = state;
}

const char IMAPConnection::hierarchySeparator() const
{
	return (m_hierarchySeparator);
}


ref <const IMAPTag> IMAPConnection::getTag() const
{
	return (m_tag);
}


ref <const IMAPParser> IMAPConnection::getParser() const
{
	return (m_parser);
}


weak_ref <const IMAPStore> IMAPConnection::getStore() const
{
	return (m_store);
}


weak_ref <IMAPStore> IMAPConnection::getStore()
{
	return (m_store);
}


ref <session> IMAPConnection::getSession()
{
	return (m_store->getSession());
}


} // imap
} // net
} // vmime
