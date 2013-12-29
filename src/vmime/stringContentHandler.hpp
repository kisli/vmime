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

#ifndef VMIME_STRINGCONTENTHANDLER_HPP_INCLUDED
#define VMIME_STRINGCONTENTHANDLER_HPP_INCLUDED


#include "vmime/contentHandler.hpp"


namespace vmime
{


class VMIME_EXPORT stringContentHandler : public contentHandler
{
public:

	stringContentHandler();
	stringContentHandler(const string& buffer, const vmime::encoding& enc = NO_ENCODING);
	stringContentHandler(const utility::stringProxy& str, const vmime::encoding& enc = NO_ENCODING);
	stringContentHandler(const string& buffer, const size_t start, const size_t end, const vmime::encoding& enc = NO_ENCODING);

	~stringContentHandler();

	stringContentHandler(const stringContentHandler& cts);
	stringContentHandler& operator=(const stringContentHandler& cts);

	shared_ptr <contentHandler> clone() const;

	// Set the data contained in the body.
	//
	// The two first functions take advantage of the COW (copy-on-write) system that
	// might be implemented into std::string. This is done using "stringProxy" object.
	//
	// Set "enc" parameter to anything other than NO_ENCODING if the data managed by
	// this content handler is already encoded with the specified encoding (so, no
	// encoding/decoding will be performed on generate()/extract()). Note that the
	// data may be re-encoded (that is, decoded and encoded) if the encoding passed
	// to generate() is different from this one...
	void setData(const utility::stringProxy& str, const vmime::encoding& enc = NO_ENCODING);
	void setData(const string& buffer, const vmime::encoding& enc = NO_ENCODING);
	void setData(const string& buffer, const size_t start, const size_t end, const vmime::encoding& enc = NO_ENCODING);

	stringContentHandler& operator=(const string& buffer);

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

	// The actual data
	utility::stringProxy m_string;
};


} // vmime


#endif // VMIME_STRINGCONTENTHANDLER_HPP_INCLUDED
