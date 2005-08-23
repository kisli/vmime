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

#ifndef VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/authenticator.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/session.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPTag;
class IMAPStore;


class IMAPConnection : public object
{
public:

	IMAPConnection(weak_ref <IMAPStore> store, ref <authenticator> auth);
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

	const ProtocolStates state() const;
	void setState(const ProtocolStates state);


	const char hierarchySeparator() const;


	void send(bool tag, const string& what, bool end);
	void sendRaw(const char* buffer, const int count);

	IMAPParser::response* readResponse(IMAPParser::literalHandler* lh = NULL);


	ref <const IMAPTag> getTag() const;
	ref <const IMAPParser> getParser() const;

	weak_ref <const IMAPStore> getStore() const;
	weak_ref <IMAPStore> getStore();

	ref <session> getSession();

private:

	weak_ref <IMAPStore> m_store;

	ref <authenticator> m_auth;

	ref <socket> m_socket;

	ref <IMAPParser> m_parser;

	ref <IMAPTag> m_tag;

	char m_hierarchySeparator;

	ProtocolStates m_state;

	ref <timeoutHandler> m_timeoutHandler;


	void internalDisconnect();

	void initHierarchySeparator();
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
