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

#ifndef VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED
#define VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED


#include "vmime/types.hpp"

#include "vmime/security/cert/certificateChain.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Verify that a certificate path issued by a server can be trusted.
  */
class certificateVerifier : public object
{
public:

	/** Verify that the specified certificate chain is trusted.
	  *
	  * @param chain certificate chain
	  * @throw exceptions::certificate_verification_exception if one
	  * or more certificates can not be trusted
	  */
	virtual void verify(ref <certificateChain> chain) = 0;
};


} // cert
} // security
} // vmime


#endif // VMIME_SECURITY_CERT_CERTIFICATEVERIFIER_HPP_INCLUDED

