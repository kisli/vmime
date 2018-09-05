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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/X509Certificate.hpp"

#include "vmime/security/cert/certificateNotYetValidException.hpp"
#include "vmime/security/cert/certificateExpiredException.hpp"


namespace vmime {
namespace security {
namespace cert {


X509Certificate::~X509Certificate() {

}


void X509Certificate::checkValidity() {

	const datetime now = datetime::now();

	if (now < getActivationDate()) {

		certificateNotYetValidException ex;
		ex.setCertificate(dynamicCast <certificate>(shared_from_this()));

		throw ex;

	} else if (now > getExpirationDate()) {

		certificateExpiredException ex;
		ex.setCertificate(dynamicCast <certificate>(shared_from_this()));

		throw ex;
	}
}


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT
