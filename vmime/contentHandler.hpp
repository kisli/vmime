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

#ifndef VMIME_CONTENTHANDLER_HPP_INCLUDED
#define VMIME_CONTENTHANDLER_HPP_INCLUDED


#include <limits>

#include "vmime/base.hpp"
#include "vmime/utility/stringProxy.hpp"
#include "vmime/utility/smartPtr.hpp"
#include "vmime/utility/progressListener.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


class contentHandler : public object
{
public:

	/** Used to specify that enclosed data is not encoded. */
	static const vmime::encoding NO_ENCODING;


	virtual ~contentHandler();

	/** Return a copy of this object.
	  *
	  * @return copy of this object
	  */
	virtual ref <contentHandler> clone() const = 0;

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
	  * @throw exceptions::no_encoder_available if the encoding is
	  * not supported
	  * @param os output stream
	  * @param progress progress listener, or NULL if you do not
	  * want to receive progress notifications
	  */
	virtual void extract(utility::outputStream& os, utility::progressListener* progress = NULL) const = 0;

	/** Extract the contents into the specified stream, without
	  * decoding it. It may be useful in case the encoding is not
	  * supported and you want to extract raw data.
	  *
	  * @param os output stream
	  * @param progress progress listener, or NULL if you do not
	  * want to receive progress notifications
	  */
	virtual void extractRaw(utility::outputStream& os, utility::progressListener* progress = NULL) const = 0;

	/** Returns the actual length of data. WARNING: this can return 0 if no
	  * length was specified when setting data of this object, or if the
	  * length is not known).
	  *
	  * @return length of data
	  */
	virtual string::size_type getLength() const = 0;

	/** Returns 'true' if data managed by this object is encoded.
	  *
	  * @return true if data is encoded, false otherwise
	  */
	virtual bool isEncoded() const = 0;

	/** Returns the encoding used for data (or "binary" if not encoded).
	  *
	  * @return encoding used for data
	  */
	virtual const vmime::encoding& getEncoding() const = 0;

	/** Returns 'true' if there is no data set.
	  *
	  * @return true if no data is managed by this object, false otherwise
	  */
	virtual bool isEmpty() const = 0;

	/** Indicates whether the extract() method can be called multiple times.
	  *
	  * @return true if the data can be extracted multiple times, or false
	  * if not (ie. streamed data from socket)
	  */
	virtual bool isBuffered() const = 0;
};


} // vmime


#endif // VMIME_CONTENTHANDLER_HPP_INCLUDED
