//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED
#define VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED


#include "vmime/contentHandler.hpp"


namespace vmime
{


class streamContentHandler : public contentHandler
{
public:

	streamContentHandler();
	streamContentHandler(utility::inputStream* is, const utility::stream::size_type length, const bool own, const vmime::encoding& enc = NO_ENCODING);

	~streamContentHandler();

	streamContentHandler(const streamContentHandler& cts);
	streamContentHandler& operator=(const streamContentHandler& cts);

	contentHandler* clone() const;

	void setData(utility::inputStream* is, const utility::stream::size_type length, const bool own, const vmime::encoding& enc = NO_ENCODING);


	void generate(utility::outputStream& os, const vmime::encoding& enc, const string::size_type maxLineLength = lineLengthLimits::infinite) const;

	void extract(utility::outputStream& os) const;

	const string::size_type getLength() const;

	const bool isEncoded() const;

	const vmime::encoding& getEncoding() const;

	const bool isEmpty() const;

private:

	// Equals to NO_ENCODING if data is not encoded, otherwise this
	// specifies the encoding that have been used to encode the data.
	vmime::encoding m_encoding;

	// Actual data
	utility::smart_ptr <utility::inputStream> m_ownedStream;   // 'contentHandler' objects are copiable...
	utility::inputStream* m_stream;
	string::size_type m_length;
};


} // vmime


#endif // VMIME_STREAMCONTENTHANDLER_HPP_INCLUDED
