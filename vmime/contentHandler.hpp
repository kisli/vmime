//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_CONTENTHANDLER_HPP_INCLUDED
#define VMIME_CONTENTHANDLER_HPP_INCLUDED


#include <limits>

#include "vmime/base.hpp"
#include "vmime/utility/stringProxy.hpp"
#include "vmime/utility/smartPtr.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


class contentHandler
{
public:

	/** Used to specify that enclosed data is not encoded. */
	static const vmime::encoding NO_ENCODING;


	virtual ~contentHandler();

	/** Return a copy of this object.
	  *
	  * @return copy of this object
	  */
	virtual contentHandler* clone() const = 0;

	/** Output the contents into the specified stream. Data will be
	  * encoded before being written into the stream. This is used internally
	  * by the body object to generate the message, you may not need to use
	  * this (see contentHandler::extract() if you want to get the contents).
	  *
	  * @param os output stream
	  * @param enc encoding for output
	  * @param maxLineLength maximum line length for output
	  */
	virtual void generate(utility::outputStream& os, const vmime::encoding& enc, const string::size_type maxLineLength = lineLengthLimits::infinite) const = 0;

	/** Extract the contents into the specified stream. If needed, data
	  * will be decoded before being written into the stream.
	  *
	  * @param os output stream
	  */
	virtual void extract(utility::outputStream& os) const = 0;

	/** Returns the actual length of data. WARNING: this can return 0 if no
	  * length was specified when setting data of this object.
	  *
	  * @return length of data
	  */
	virtual const string::size_type getLength() const = 0;

	/** Returns 'true' if data managed by this object is encoded.
	  *
	  * @return true if data is encoded, false otherwise
	  */
	virtual const bool isEncoded() const = 0;

	/** Returns the encoding used for data (or "binary" if not encoded).
	  *
	  * @return encoding used for data
	  */
	virtual const vmime::encoding& getEncoding() const = 0;

	/** Returns 'true' if there is no data set.
	  *
	  * @return true if no data is managed by this object, false otherwise
	  */
	virtual const bool isEmpty() const = 0;
};


} // vmime


#endif // VMIME_CONTENTHANDLER_HPP_INCLUDED
