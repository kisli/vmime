//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/messaging/IMAPTag.hpp"
#include "vmime/messaging/IMAPConnection.hpp"
#include "vmime/messaging/IMAPUtils.hpp"
#include "vmime/messaging/IMAPStore.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include <sstream>


namespace vmime {
namespace messaging {


IMAPConnection::IMAPConnection(IMAPStore* store, authenticator* auth)
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

	delete (m_tag);
	delete (m_parser);
}


void IMAPConnection::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_state = STATE_NONE;
	m_hierarchySeparator = '\0';

	const string address = m_store->getSession()->getProperties()[m_store->getInfos().getPropertyPrefix() + "server.address"];
	const port_t port = m_store->getSession()->getProperties().getProperty(m_store->getInfos().getPropertyPrefix() + "server.port", m_store->getInfos().getDefaultPort());

	// Create the time-out handler
	if (m_store->getSession()->getProperties().hasProperty
		(m_store->getInfos().getPropertyPrefix() + "timeout.factory"))
	{
		timeoutHandlerFactory* tof = platformDependant::getHandler()->
			getTimeoutHandlerFactory(m_store->getSession()->getProperties()
				[m_store->getInfos().getPropertyPrefix() + "timeout.factory"]);

		m_timeoutHandler = tof->create();
	}

	// Create and connect the socket
	socketFactory* sf = platformDependant::getHandler()->getSocketFactory
		(m_store->getSession()->getProperties().getProperty
			(m_store->getInfos().getPropertyPrefix() + "server.socket-factory", string("default")));

	m_socket = sf->create();
	m_socket->connect(address, port);

	delete (m_tag);
	m_tag = new IMAPTag();

	delete (m_parser);
	m_parser = new IMAPParser(m_tag, m_socket, m_timeoutHandler);


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
	send(true, "LOGOUT", true);

	m_socket->disconnect();

	delete (m_socket);
	m_socket = NULL;

	delete (m_timeoutHandler);
	m_timeoutHandler = NULL;

	m_state = STATE_LOGOUT;
}


void IMAPConnection::initHierarchySeparator()
{
	send(true, "LIST \"\" \"\"", true);

	utility::auto_ptr <IMAPParser::response> resp(m_parser->readResponse());

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


} // messaging
} // vmime
