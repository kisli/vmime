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

#ifndef VMIME_UTILITY_ENCODER_ENCODER_HPP_INCLUDED
#define VMIME_UTILITY_ENCODER_ENCODER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/propertySet.hpp"
#include "vmime/exception.hpp"
#include "vmime/utility/progressListener.hpp"


namespace vmime {
namespace utility {
namespace encoder {


/** Encode/decode data in different encodings.
  */

class VMIME_EXPORT encoder : public object
{
public:

	encoder();
	virtual ~encoder();

	/** Encode data.
	  *
	  * @param in input data (decoded)
	  * @param out output stream for encoded data
	  * @param progress progress listener, or NULL if you do not
	  * want to receive progress notifications
	  * @return number of bytes written into output stream
	  */
	virtual size_t encode(utility::inputStream& in, utility::outputStream& out, utility::progressListener* progress = NULL) = 0;

	/** Decode data.
	  *
	  * @param in input data (encoded)
	  * @param out output stream for decoded data
	  * @param progress progress listener, or NULL if you do not
	  * want to receive progress notifications
	  * @return number of bytes written into output stream
	  */
	virtual size_t decode(utility::inputStream& in, utility::outputStream& out, utility::progressListener* progress = NULL) = 0;

	/** Return the properties of the encoder.
	  *
	  * @return properties of the encoder
	  */
	const propertySet& getProperties() const;

	/** Return the properties of the encoder.
	  *
	  * @return properties of the encoder
	  */
	propertySet& getProperties();

	/** Return a list of property names that can be set for
	  * this encoder.
	  *
	  * @return list of property names
	  */
	virtual const std::vector <string> getAvailableProperties() const;

	/** Return the results returned by this encoder.
	  *
	  * @return results returned by the encoder
	  */
	const propertySet& getResults() const;

	/** Return the encoded size for the specified input (decoded) size.
	  * If the size is not exact, it may be an estimate which should always
	  * be larger than the actual encoded size.
	  *
	  * @param n count of input (decoded) bytes
	  * @return count of output (encoded) bytes
	  */
	virtual size_t getEncodedSize(const size_t n) const = 0;

	/** Return the encoded size for the specified input (encoded) size.
	  * If the size is not exact, it may be an estimate which should always
	  * be larger than the actual decoded size.
	  *
	  * @param n count of input (encoded) bytes
	  * @return count of output (decoded) bytes
	  */
	virtual size_t getDecodedSize(const size_t n) const = 0;

protected:

	propertySet& getResults();

private:

	propertySet m_props;
	propertySet m_results;
};


} // encoder
} // utility
} // vmime


#endif // VMIME_UTILITY_ENCODER_ENCODER_HPP_INCLUDED
