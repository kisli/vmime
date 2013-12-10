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

#ifndef VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED
#define VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED


#include "vmime/contentHandler.hpp"


namespace vmime
{


/** A content handler which obtains its data from a stream.
  */

class VMIME_EXPORT streamContentHandler : public contentHandler
{
public:

	/** Creates a new empty content handler. No data can be extracted until
	  * an input stream is set using setData() function.
	  *
	  * @return a reference to a new content handler
	  */
	streamContentHandler();

	/** Creates a new content handler using an input stream.
	  *
	  * @param is input stream from which data will be obtained
	  * @param length expected stream length. May be zero, but it is highly
	  * recommended to set this parameter to take part of some optimizations
	  * and features (eg. SMTP CHUNKING/SIZE extension).
	  * @param enc set to anything other than NO_ENCODING if the data obtained
	  * from the stream is already encoded with the specified encoding
	  *
	  * @return a reference to a new content handler
	  */
	streamContentHandler
		(shared_ptr <utility::inputStream> is,
		 const size_t length,
		 const vmime::encoding& enc = NO_ENCODING);

	~streamContentHandler();

	streamContentHandler(const streamContentHandler& cts);
	streamContentHandler& operator=(const streamContentHandler& cts);

	shared_ptr <contentHandler> clone() const;

	/** Sets the data managed by this content handler.
	  *
	  * @param is input stream from which data will be obtained
	  * @param length expected stream length. May be zero, but it is highly
	  * recommended to set this parameter to take part of some optimizations
	  * and features (eg. SMTP CHUNKING/SIZE extension).
	  * @param enc set to anything other than NO_ENCODING if the data obtained
	  * from the stream is already encoded with the specified encoding
	  */
	void setData
		(shared_ptr <utility::inputStream> is,
		 const size_t length,
		 const vmime::encoding& enc = NO_ENCODING);


	void generate(utility::outputStream& os, const vmime::encoding& enc, const size_t maxLineLength = lineLengthLimits::infinite) const;

	void extract(utility::outputStream& os, utility::progressListener* progress = NULL) const;
	void extractRaw(utility::outputStream& os, utility::progressListener* progress = NULL) const;

	size_t getLength() const;

	bool isEncoded() const;

	const vmime::encoding& getEncoding() const;

	bool isEmpty() const;

	bool isBuffered() const;

	void setContentTypeHint(const mediaType& type);
	const mediaType getContentTypeHint() const;

private:

	mediaType m_contentType;

	// Equals to NO_ENCODING if data is not encoded, otherwise this
	// specifies the encoding that have been used to encode the data.
	vmime::encoding m_encoding;

	// Actual data
	mutable shared_ptr <utility::inputStream> m_stream;
	size_t m_length;
};


} // vmime


#endif // VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED
