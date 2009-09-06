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

#ifndef VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED


#include "vmime/types.hpp"

#include "vmime/net/tls/TLSSocket.hpp"

#include "vmime/security/cert/certificateVerifier.hpp"


namespace vmime {
namespace net {
namespace tls {


/** Describe a TLS connection between a client and a server.
  */
class TLSSession : public object
{
	friend class TLSSocket;

public:

	~TLSSession();

	/** Create and initialize a new TLS session.
	  *
	  * @param cv object responsible for verifying certificates
	  * sent by the server
	  * @return a new TLS session
	  */
	TLSSession(ref <security::cert::certificateVerifier> cv);

	/** Create a new socket that adds a TLS security layer around
	  * an existing socket. You should create only one socket
	  * per session.
	  *
	  * @param sok socket to wrap
	  * @return TLS socket wrapper
	  */
	ref <TLSSocket> getSocket(ref <socket> sok);

	/** Get the object responsible for verifying certificates when
	  * using secured connections (TLS/SSL).
	  */
	ref <security::cert::certificateVerifier> getCertificateVerifier();

private:

	TLSSession(const TLSSession&);

	static void throwTLSException(const string& fname, const int code);


#ifdef LIBGNUTLS_VERSION
	gnutls_session* m_gnutlsSession;
#else
	void* m_gnutlsSession;
#endif // LIBGNUTLS_VERSION

	ref <security::cert::certificateVerifier> m_certVerifier;
};


} // tls
} // net
} // vmime


#endif // VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED

