//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_ENCODER_HPP_INCLUDED
#define VMIME_ENCODER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/propertySet.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


/** Encode/decode data in different encodings.
  */

class encoder
{
public:

	encoder();
	virtual ~encoder();

	/** Encode data.
	  *
	  * @param in input data (decoded)
	  * @param out output stream for encoded data
	  * @return number of bytes written into output stream
	  */
	virtual const utility::stream::size_type encode(utility::inputStream& in, utility::outputStream& out) = 0;

	/** Decode data.
	  *
	  * @param in input data (encoded)
	  * @param out output stream for decoded data
	  * @return number of bytes written into output stream
	  */
	virtual const utility::stream::size_type decode(utility::inputStream& in, utility::outputStream& out) = 0;

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

protected:

	propertySet& getResults();

private:

	propertySet m_props;
	propertySet m_results;
};


} // vmime


#endif // VMIME_ENCODER_HPP_INCLUDED
