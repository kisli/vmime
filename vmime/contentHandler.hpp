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
private:

	static const vmime::encoding NO_ENCODING;

public:

	contentHandler();
	contentHandler(const string& buffer, const vmime::encoding& enc = NO_ENCODING);  // for compatibility
	~contentHandler();

	// Copy
	contentHandler(const contentHandler& cts);
	contentHandler& operator=(const contentHandler& cts);

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
	//
	// The 'length' parameter is optional (user-defined). You can pass 0 if you want,
	// VMime does not make use of it.
	void setData(const utility::stringProxy& str, const vmime::encoding& enc = NO_ENCODING);
	void setData(const string& buffer, const vmime::encoding& enc = NO_ENCODING);
	void setData(const string& buffer, const string::size_type start, const string::size_type end, const vmime::encoding& enc = NO_ENCODING);
	void setData(utility::inputStream* const is, const utility::stream::size_type length, const bool own, const vmime::encoding& enc = NO_ENCODING);

	// For compatibility
	contentHandler& operator=(const string& buffer);

	// WRITE: Output the contents into the specified stream. Data will be
	// encoded before being written into the stream. This is used internally
	// by the body object to generate the message, you may not need to use
	// this (see function extract() if you want to get the contents).
	void generate(utility::outputStream& os, const vmime::encoding& enc, const string::size_type maxLineLength = lineLengthLimits::infinite) const;

	// READ: Extract the contents into the specified stream. If needed, data
	// will be decoded before being written into the stream.
	void extract(utility::outputStream& os) const;

	// Returns the actual length of the data. WARNING: this can return 0 if no
	// length was specified when setting data of this object.
	const string::size_type getLength() const;

	// Returns 'true' if the data managed by this object is encoded.
	const bool isEncoded() const;

	// Returns the encoding used for the data (or "binary" if not encoded).
	const vmime::encoding& getEncoding() const;

	// Returns 'true' if there is no data set.
	const bool isEmpty() const;

private:

	// Source of data managed by this content handler
	enum Types
	{
		TYPE_NONE,
		TYPE_STRING,
		TYPE_STREAM
	};

	Types m_type;

	// Equals to NO_ENCODING if data is not encoded, otherwise this
	// specifies the encoding that have been used to encode the data.
	vmime::encoding m_encoding;

	// Used if m_type == TYPE_STRING
	utility::stringProxy m_string;

	// Used if m_type == TYPE_STREAM
	utility::smart_ptr <utility::inputStream> m_ownedStream;   // 'contentHandler' objects are copiable...
	utility::inputStream* m_stream;
	string::size_type m_length;
};


} // vmime


#endif // VMIME_CONTENTHANDLER_HPP_INCLUDED
