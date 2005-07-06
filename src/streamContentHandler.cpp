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

#include "vmime/streamContentHandler.hpp"


namespace vmime
{


streamContentHandler::streamContentHandler()
	: m_encoding(NO_ENCODING), m_ownedStream(NULL), m_stream(NULL)
{
}


streamContentHandler::streamContentHandler(utility::inputStream* is,
	const utility::stream::size_type length, const bool own, const vmime::encoding& enc)
{
	setData(is, length, own, enc);
}


streamContentHandler::~streamContentHandler()
{
}


streamContentHandler::streamContentHandler(const streamContentHandler& cts)
	: contentHandler(), m_encoding(cts.m_encoding),
	  m_ownedStream(const_cast <utility::smart_ptr <utility::inputStream>&>(cts.m_ownedStream)),
	  m_stream(cts.m_stream), m_length(cts.m_length)
{
}


contentHandler* streamContentHandler::clone() const
{
	return new streamContentHandler(*this);
}


streamContentHandler& streamContentHandler::operator=(const streamContentHandler& cts)
{
	m_encoding = cts.m_encoding;

	m_ownedStream = const_cast <utility::smart_ptr <utility::inputStream>&>(cts.m_ownedStream);
	m_stream = cts.m_stream;
	m_length = cts.m_length;

	return (*this);
}


void streamContentHandler::setData(utility::inputStream* is,
	const utility::stream::size_type length, const bool own, const vmime::encoding& enc)
{
	m_encoding = enc;
	m_length = length;

	if (own)
	{
		m_ownedStream = is;
		m_stream = NULL;
	}
	else
	{
		m_ownedStream = NULL;
		m_stream = is;
	}
}


void streamContentHandler::generate(utility::outputStream& os, const vmime::encoding& enc,
	const string::size_type maxLineLength) const
{
	if (m_stream == NULL && m_ownedStream.ptr() == NULL)
		return;

	// Managed data is already encoded
	if (isEncoded())
	{
		// The data is already encoded but the encoding specified for
		// the generation is different from the current one. We need
		// to re-encode data: decode from input buffer to temporary
		// buffer, and then re-encode to output stream...
		if (m_encoding != enc)
		{
			utility::auto_ptr <encoder> theDecoder(m_encoding.getEncoder());
			utility::auto_ptr <encoder> theEncoder(enc.getEncoder());

			theEncoder->getProperties()["maxlinelength"] = maxLineLength;

			utility::inputStream& in = const_cast <utility::inputStream&>
				(*(m_stream ? m_stream : m_ownedStream.ptr()));

			in.reset();  // may not work...

			std::ostringstream oss;
			utility::outputStreamAdapter tempOut(oss);

			theDecoder->decode(in, tempOut);

			string str = oss.str();
			utility::inputStreamStringAdapter tempIn(str);

			theEncoder->encode(tempIn, os);
		}
		// No encoding to perform
		else
		{
			utility::inputStream& in = const_cast <utility::inputStream&>
				(*(m_stream ? m_stream : m_ownedStream.ptr()));

			in.reset();  // may not work...

			utility::bufferedStreamCopy(in, os);
		}
	}
	// Need to encode data before
	else
	{
		utility::auto_ptr <encoder> theEncoder(enc.getEncoder());
		theEncoder->getProperties()["maxlinelength"] = maxLineLength;

		utility::inputStream& in = const_cast <utility::inputStream&>
			(*(m_stream ? m_stream : m_ownedStream.ptr()));

		in.reset();  // may not work...

		theEncoder->encode(in, os);
	}
}


void streamContentHandler::extract(utility::outputStream& os) const
{
	if (m_stream == NULL && m_ownedStream.ptr() == NULL)
		return;

	// No decoding to perform
	if (!isEncoded())
	{
		utility::inputStream& in = const_cast <utility::inputStream&>
			(*(m_stream ? m_stream : m_ownedStream.ptr()));

		in.reset();  // may not work...

		utility::bufferedStreamCopy(in, os);
	}
	// Need to decode data
	else
	{
		utility::auto_ptr <encoder> theDecoder(m_encoding.getEncoder());

		utility::inputStream& in = const_cast <utility::inputStream&>
			(*(m_stream ? m_stream : m_ownedStream.ptr()));

		in.reset();  // may not work...

		theDecoder->decode(in, os);
	}
}


void streamContentHandler::extractRaw(utility::outputStream& os) const
{
	if (m_stream == NULL && m_ownedStream.ptr() == NULL)
		return;

	utility::inputStream& in = const_cast <utility::inputStream&>
		(*(m_stream ? m_stream : m_ownedStream.ptr()));

	in.reset();  // may not work...

	utility::bufferedStreamCopy(in, os);
}


const string::size_type streamContentHandler::getLength() const
{
	return (m_length);
}


const bool streamContentHandler::isEmpty() const
{
	return (m_length == 0 || (m_stream == NULL && m_ownedStream.ptr() == NULL));
}


const bool streamContentHandler::isEncoded() const
{
	return (m_encoding != NO_ENCODING);
}


const vmime::encoding& streamContentHandler::getEncoding() const
{
	return (m_encoding);
}


} // vmime
