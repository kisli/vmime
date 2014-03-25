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

#ifndef VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


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
class VMIME_EXPORT TLSSocket : public socket
{
public:

	/** Create a new socket object that adds a security layer
	  * around an existing socket.
	  *
	  * @param session TLS session
	  * @param sok socket to wrap
	  */
	static shared_ptr <TLSSocket> wrap(shared_ptr <TLSSession> session, shared_ptr <socket> sok);

	/** Starts a TLS handshake on this connection.
	  *
	  * @throw exceptions::tls_exception if a fatal error occurs
	  * during the negociation process, exceptions::operation_timed_out
	  * if a time-out occurs
	  */
	virtual void handshake() = 0;

	/** Return the peer's certificate (chain) as sent by the peer.
	  *
	  * @return server certificate chain, or NULL if the handshake
	  * has not been performed yet
	  */
	virtual shared_ptr <security::cert::certificateChain> getPeerCertificates() = 0;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_NET_TLS_TLSSOCKET_HPP_INCLUDED
