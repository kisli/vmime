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

#ifndef VMIME_MESSAGING_IMAPCONNECTION_HPP_INCLUDED
#define VMIME_MESSAGING_IMAPCONNECTION_HPP_INCLUDED


#include "authenticator.hpp"
#include "socket.hpp"
#include "../config.hpp"
#include "timeoutHandler.hpp"

#include "IMAPParser.hpp"


namespace vmime {
namespace messaging {


class IMAPTag;
class IMAPStore;


class IMAPConnection
{
public:

	IMAPConnection(IMAPStore* store, authenticator* auth);
	~IMAPConnection();


	void connect();
	const bool isConnected() const;
	void disconnect();


	enum ProtocolStates
	{
		STATE_NONE,
		STATE_NON_AUTHENTICATED,
		STATE_AUTHENTICATED,
		STATE_SELECTED,
		STATE_LOGOUT
	};

	const ProtocolStates state() const { return (m_state); }
	void setState(const ProtocolStates state) { m_state = state; }


	const char hierarchySeparator() const { return (m_hierarchySeparator); }


	void send(bool tag, const string& what, bool end);
	void sendRaw(const char* buffer, const int count);

	IMAPParser::response* readResponse(IMAPParser::literalHandler* lh = NULL);


	const IMAPTag* getTag() const { return (m_tag); }
	const IMAPParser* getParser() const { return (m_parser); }

	const IMAPStore* getStore() const { return (m_store); }
	IMAPStore* getStore() { return (m_store); }

private:

	IMAPStore* m_store;

	authenticator* m_auth;

	socket* m_socket;

	IMAPParser* m_parser;

	IMAPTag* m_tag;

	char m_hierarchySeparator;

	ProtocolStates m_state;

	timeoutHandler* m_timeoutHandler;


	void internalDisconnect();

	void initHierarchySeparator();
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_IMAPCONNECTION_HPP_INCLUDED
