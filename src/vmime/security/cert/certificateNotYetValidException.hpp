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

#ifndef VMIME_SECURITY_CERT_CERTIFICATENOTYETVALIDEXCEPTION_HPP_INCLUDED
#define VMIME_SECURITY_CERT_CERTIFICATENOTYETVALIDEXCEPTION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/certificate.hpp"
#include "vmime/security/cert/certificateException.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Thrown when the current date and time is before the validity period
  * specified in the certificate.
  */
class VMIME_EXPORT certificateNotYetValidException : public certificateException {

public:

	/** Constructs a certificateNotYetValidException with no detail message.
	  */
	certificateNotYetValidException();

	exception* clone() const;
};


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_SECURITY_CERT_CERTIFICATENOTYETVALIDEXCEPTION_HPP_INCLUDED
