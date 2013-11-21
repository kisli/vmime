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

#ifndef VMIME_NET_TLS_TLSSESSION_GNUTLS_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSESSION_GNUTLS_HPP_INCLUDED


#ifndef VMIME_BUILDING_DOC


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "vmime/types.hpp"

#include "vmime/net/tls/TLSSession.hpp"
#include "vmime/net/tls/TLSSocket.hpp"
#include "vmime/net/tls/TLSProperties.hpp"


namespace vmime {
namespace net {
namespace tls {


class TLSSession_GnuTLS : public TLSSession
{
	friend class TLSSocket_GnuTLS;

public:

	TLSSession_GnuTLS(shared_ptr <security::cert::certificateVerifier> cv, shared_ptr <TLSProperties> props);
	~TLSSession_GnuTLS();


	shared_ptr <TLSSocket> getSocket(shared_ptr <socket> sok);

	shared_ptr <security::cert::certificateVerifier> getCertificateVerifier();

private:

	TLSSession_GnuTLS(const TLSSession_GnuTLS&);

	static void throwTLSException(const string& fname, const int code);


#ifdef LIBGNUTLS_VERSION
	gnutls_session* m_gnutlsSession;
#else
	void* m_gnutlsSession;
#endif // LIBGNUTLS_VERSION

	shared_ptr <security::cert::certificateVerifier> m_certVerifier;
	shared_ptr <TLSProperties> m_props;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS

#endif // VMIME_BUILDING_DOC

#endif // VMIME_NET_TLS_TLSSESSION_GNUTLS_HPP_INCLUDED

