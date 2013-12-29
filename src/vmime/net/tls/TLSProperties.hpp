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

#ifndef VMIME_NET_TLS_TLSPROPERTIES_HPP_INCLUDED
#define VMIME_NET_TLS_TLSPROPERTIES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/types.hpp"


namespace vmime {
namespace net {
namespace tls {


/** Holds options for a TLS session.
  */
class VMIME_EXPORT TLSProperties : public object
{
public:

	TLSProperties();
	TLSProperties(const TLSProperties&);


	/** Predefined generic cipher suites (work with all TLS libraries). */
	enum GenericCipherSuite
	{
		CIPHERSUITE_HIGH,           /**< High encryption cipher suites (> 128 bits). */
		CIPHERSUITE_MEDIUM,         /**< Medium encryption cipher suites (>= 128 bits). */
		CIPHERSUITE_LOW,            /**< Low encryption cipher suites (>= 64 bits). */

		CIPHERSUITE_DEFAULT         /**< Default cipher suite. */
	};

	/** Sets the cipher suite preferences for a SSL/TLS session, using
	  * predefined, generic suites. This works with all underlying TLS
	  * libraries (OpenSSL and GNU TLS).
	  *
	  * @param cipherSuite predefined cipher suite
	  */
	void setCipherSuite(const GenericCipherSuite cipherSuite);

	/** Sets the cipher suite preferences for a SSL/TLS session, using
	  * a character string. The format and meaning of the string depend
	  * on the underlying TLS library.
	  *
	  * For GNU TLS, read this:
	  * http://gnutls.org/manual/html_node/Priority-Strings.html
	  *
	  * For OpenSSL, read this:
	  * http://www.openssl.org/docs/apps/ciphers.html#CIPHER_STRINGS
	  *
	  * @param cipherSuite cipher suite as a string
	  */
	void setCipherSuite(const string& cipherSuite);

	/** Returns the cipher suite preferences for a SSL/TLS session, as
	  * a character string. The format and meaning of the string depend
	  * on the underlying TLS library (see setCipherSuite() method).
	  *
	  * @return cipher suite string
	  */
	const string getCipherSuite() const;

private:

	shared_ptr <object> m_data;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_NET_TLS_TLSPROPERTIES_HPP_INCLUDED
