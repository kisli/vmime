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

#ifndef VMIME_SECURITY_CERT_CERTIFICATE_HPP_INCLUDED
#define VMIME_SECURITY_CERT_CERTIFICATE_HPP_INCLUDED


#include "vmime/types.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Identity certificate for a peer.
  */
class VMIME_EXPORT certificate : public object
{
public:

	/** Returns the encoded form of this certificate (for example,
	  * X.509 certificates are encoded as ASN.1 DER).
	  *
	  * @return the encoded form of this certificate
	  */
	virtual const byteArray getEncoded() const = 0;

	/** Return the type of this certificate.
	  *
	  * @return the type of this certificate
	  */
	virtual const string getType() const = 0;

	/** Return the version of this certificate.
	  *
	  * @return the version of this certificate
	  */
	virtual int getVersion() const = 0;

	/** Checks if two certificates are the same.
	  *
	  * @param other certificate to compare with
	  * @return true if the two certificates are the same,
	  * false otherwise
	  */
	virtual bool equals(shared_ptr <const certificate> other) const = 0;

	/** Returns a pointer to internal binary data for this certificate.
	  * The actual type of data depends on the library used for TLS support.
	  *
	  * @return pointer to underlying data
	  */
	virtual void* getInternalData() = 0;
};


} // cert
} // security
} // vmime


#endif // VMIME_SECURITY_CERT_CERTIFICATE_HPP_INCLUDED

