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

#ifndef VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED


#include "vmime/exception.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/security/cert/certificateChain.hpp"


namespace vmime {
namespace net {
namespace tls {


class TLSSession;


/** Add a TLS security layer to an existing socket.
  */
class TLSSocket : public socket
{
	friend class vmime::creator;

protected:

	/** Create a new socket object that adds a security layer
	  * around an existing socket.
	  *
	  * @param session TLS session
	  * @param sok socket to wrap
	  */
	TLSSocket(ref <TLSSession> session, ref <socket> sok);

public:

	~TLSSocket();


	/** Starts a TLS handshake on this connection.
	  *
	  * @throw exceptions::tls_exception if a fatal error occurs
	  * during the negociation process, exceptions::operation_timed_out
	  * if a time-out occurs
	  */
	void handshake(ref <timeoutHandler> toHandler = NULL);

	/** Return the peer's certificate (chain) as sent by the peer.
	  *
	  * @return server certificate chain, or NULL if the handshake
	  * has not been performed yet
	  */
	ref <security::cert::certificateChain> getPeerCertificates() const;


	// Implementation of 'socket'
	void connect(const string& address, const port_t port);
	void disconnect();
	bool isConnected() const;

	void receive(string& buffer);
	size_type receiveRaw(char* buffer, const size_type count);

	void send(const string& buffer);
	void sendRaw(const char* buffer, const size_type count);

	size_type getBlockSize() const;

private:

	void internalThrow();

#ifdef LIBGNUTLS_VERSION
	static ssize_t gnutlsPushFunc(gnutls_transport_ptr trspt, const void* data, size_t len);
	static ssize_t gnutlsPullFunc(gnutls_transport_ptr trspt, void* data, size_t len);
#else
	static int gnutlsPushFunc(void* trspt, const void* data, size_t len);
	static int gnutlsPullFunc(void* trspt, void* data, size_t len);
#endif // LIBGNUTLS_VERSION


	ref <TLSSession> m_session;
	ref <socket> m_wrapped;

	bool m_connected;

	char m_buffer[65536];

	bool m_handshaking;
	ref <timeoutHandler> m_toHandler;

	exception* m_ex;
};


} // tls
} // net
} // vmime


#endif // VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED

