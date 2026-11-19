//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_SECURITY_CERT_CERTIFICATEISSUERVERIFICATIONEXCEPTION_HPP_INCLUDED
#define VMIME_SECURITY_CERT_CERTIFICATEISSUERVERIFICATIONEXCEPTION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/certificate.hpp"
#include "vmime/security/cert/certificateException.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Thrown when a certificate in a certificate chain cannot be verified
  * against its issuer: invalid signature, issuer which is not a CA or
  * is not allowed to sign certificates, path length or name constraints
  * of the issuer violated.
  */
class VMIME_EXPORT certificateIssuerVerificationException : public certificateException {

public:

	/** Constructs a certificateIssuerVerificationException with no detail message.
	  */
	certificateIssuerVerificationException();

	/** Constructs a certificateIssuerVerificationException with the precise
	  * reason of the failure.
	  *
	  * @param reason reason of the failure (eg. "certificate signature
	  * failure"), given by the TLS library
	  */
	explicit certificateIssuerVerificationException(const string& reason);

	exception* clone() const;
};


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_SECURITY_CERT_CERTIFICATEISSUERVERIFICATIONEXCEPTION_HPP_INCLUDED
