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

#ifndef VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/tracer.hpp"
#include "vmime/net/session.hpp"
#include "vmime/net/connectionInfos.hpp"

#include "vmime/net/imap/IMAPParser.hpp"

#include "vmime/security/authenticator.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPTag;
class IMAPStore;
class IMAPCommand;


class VMIME_EXPORT IMAPConnection : public object
{
public:

	IMAPConnection(shared_ptr <IMAPStore> store, shared_ptr <security::authenticator> auth);
	~IMAPConnection();


	void connect();
	bool isConnected() const;
	void disconnect();


	enum ProtocolStates
	{
		STATE_NONE,
		STATE_NON_AUTHENTICATED,
		STATE_AUTHENTICATED,
		STATE_SELECTED,
		STATE_LOGOUT
	};

	ProtocolStates state() const;
	void setState(const ProtocolStates state);


	char hierarchySeparator() const;


	void sendCommand(shared_ptr <IMAPCommand> cmd);
	void sendRaw(const byte_t* buffer, const size_t count);

	IMAPParser::response* readResponse(IMAPParser::literalHandler* lh = NULL);


	shared_ptr <const IMAPStore> getStore() const;
	shared_ptr <IMAPStore> getStore();

	shared_ptr <session> getSession();

	void fetchCapabilities();
	void invalidateCapabilities();
	const std::vector <string> getCapabilities();
	bool hasCapability(const string& capa);
	bool hasCapability(const string& capa) const;

	shared_ptr <security::authenticator> getAuthenticator();

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;

	shared_ptr <const socket> getSocket() const;
	void setSocket(shared_ptr <socket> sok);

	shared_ptr <tracer> getTracer();

	shared_ptr <IMAPTag> getTag();

	bool isMODSEQDisabled() const;
	void disableMODSEQ();

private:

	void authenticate();
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	bool processCapabilityResponseData(const IMAPParser::response* resp);
	void processCapabilityResponseData(const IMAPParser::capability_data* capaData);


	weak_ptr <IMAPStore> m_store;

	shared_ptr <security::authenticator> m_auth;

	shared_ptr <socket> m_socket;

	shared_ptr <IMAPParser> m_parser;

	shared_ptr <IMAPTag> m_tag;

	char m_hierarchySeparator;

	ProtocolStates m_state;

	shared_ptr <timeoutHandler> m_timeoutHandler;

	bool m_secured;
	shared_ptr <connectionInfos> m_cntInfos;

	bool m_firstTag;

	std::vector <string> m_capabilities;
	bool m_capabilitiesFetched;

	bool m_noModSeq;

	shared_ptr <tracer> m_tracer;


	void internalDisconnect();

	void initHierarchySeparator();
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
