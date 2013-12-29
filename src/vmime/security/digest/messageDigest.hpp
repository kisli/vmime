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

#ifndef VMIME_SECURITY_DIGEST_MESSAGEDIGEST_HPP_INCLUDED
#define VMIME_SECURITY_DIGEST_MESSAGEDIGEST_HPP_INCLUDED


#include "vmime/object.hpp"
#include "vmime/types.hpp"


namespace vmime {
namespace security {
namespace digest {


/** Computes message digests using standard algorithms,
  * such as MD5 or SHA.
  */

class VMIME_EXPORT messageDigest : public object
{
public:

	/** Updates the digest using the specified string.
	  *
	  * @param s the string with which to update the digest.
	  */
	virtual void update(const string& s) = 0;

	/** Updates the digest using the specified byte.
	  *
	  * @param b the byte with which to update the digest.
	  */
	virtual void update(const byte_t b) = 0;

	/** Updates the digest using the specified array of bytes.
	  *
	  * @param buffer array of bytes
	  * @param len number of bytes to use in the buffer
	  */
	virtual void update(const byte_t* buffer, const size_t len) = 0;

	/** Updates the digest using the specified array of bytes,
	  * starting at the specified offset.
	  *
	  * @param buffer array of bytes
	  * @param offset offset to start from in the array of bytes
	  * @param len number of bytes to use, starting at offset
	  */
	virtual void update(const byte_t* buffer,
	                    const size_t offset,
	                    const size_t len) = 0;

	/** Completes the hash computation by performing final operations
	  * such as padding.
	  */
	virtual void finalize() = 0;

	/** Completes the hash computation by performing final operations
	  * such as padding. This is equivalent to calling update() and
	  * then finalize().
	  */
	virtual void finalize(const string& s) = 0;

	/** Completes the hash computation by performing final operations
	  * such as padding. This is equivalent to calling update() and
	  * then finalize().
	  */
	virtual void finalize(const byte_t* buffer,
	                      const size_t len) = 0;

	/** Completes the hash computation by performing final operations
	  * such as padding. This is equivalent to calling update() and
	  * then finalize().
	  */
	virtual void finalize(const byte_t* buffer,
	                      const size_t offset,
	                      const size_t len) = 0;

	/** Returns the length of the hash.
	  * This is the length of the array returned by getDigest().
	  *
	  * @return length of computed hash
	  */
	virtual size_t getDigestLength() const = 0;

	/** Returns the hash, as computed by the algorithm.
	  * You must call finalize() before using this function, or the
	  * hash will not be correct.
	  * To get the size of the returned array, call getDigestLength().
	  *
	  * @return computed hash
	  */
	virtual const byte_t* getDigest() const = 0;

	/** Returns the hash as an hexadecimal string.
	  * You must call finalize() before using this function, or the
	  * hash will not be correct.
	  *
	  * @return computed hash, in hexadecimal format
	  */
	virtual const string getHexDigest() const;

	/** Resets the algorithm to its initial state, so that you can
	  * compute a new hash using the same object.
	  */
	virtual void reset() = 0;
};


} // digest
} // security
} // vmime


#endif // VMIME_SECURITY_DIGEST_MESSAGEDIGEST_HPP_INCLUDED

