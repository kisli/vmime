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

#ifndef VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED
#define VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED


#include "vmime/types.hpp"

#include "vmime/security/cert/certificateChain.hpp"

#include "vmime/security/cert/unsupportedCertificateTypeException.hpp"
#include "vmime/security/cert/certificateIssuerVerificationException.hpp"
#include "vmime/security/cert/certificateNotTrustedException.hpp"
#include "vmime/security/cert/serverIdentityException.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Verify that a certificate path issued by a server can be trusted.
  */
class VMIME_EXPORT certificateVerifier : public object {

public:

	/** Verify that the specified certificate chain is trusted.
	  *
	  * @param chain certificate chain
	  * @param hostname server hostname
	  * @throw unsupportedCertificateTypeException if a certificate in the
	  * chain is of unsupported format
	  * @throw certificateExpiredException if a certificate in the chain
	  * has expired
	  * @throw certificateNotYetValidException if a certificate in the chain
	  * is not yet valid
	  * @throw certificateNotTrustedException if a certificate in the chain
	  * cannot be verified against root and/or trusted certificates
	  * @throw certificateIssuerVerificationException if a certificate in the
	  * chain cannot be verified against the next certificate (issuer)
	  * @throw serverIdentityException if the subject name of the certificate
	  * does not match the hostname of the server
	  */
	virtual void verify(const shared_ptr <certificateChain>& chain, const string& hostname) = 0;
};


} // cert
} // security
} // vmime


#endif // VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED

