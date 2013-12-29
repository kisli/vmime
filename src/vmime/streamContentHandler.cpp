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

#include "vmime/streamContentHandler.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/seekableInputStream.hpp"
#include "vmime/utility/streamUtils.hpp"


namespace vmime
{


streamContentHandler::streamContentHandler()
	: m_encoding(NO_ENCODING), m_stream(null), m_length(0)
{
}


streamContentHandler::streamContentHandler(shared_ptr <utility::inputStream> is,
	const size_t length, const vmime::encoding& enc)
{
	setData(is, length, enc);
}


streamContentHandler::~streamContentHandler()
{
}


streamContentHandler::streamContentHandler(const streamContentHandler& cts)
	: contentHandler(), m_contentType(cts.m_contentType), m_encoding(cts.m_encoding),
	  m_stream(cts.m_stream), m_length(cts.m_length)
{
}


shared_ptr <contentHandler> streamContentHandler::clone() const
{
	return make_shared <streamContentHandler>(*this);
}


streamContentHandler& streamContentHandler::operator=(const streamContentHandler& cts)
{
	m_contentType = cts.m_contentType;
	m_encoding = cts.m_encoding;

	m_stream = cts.m_stream;
	m_length = cts.m_length;

	return (*this);
}


void streamContentHandler::setData(shared_ptr <utility::inputStream> is,
	const size_t length, const vmime::encoding& enc)
{
	m_encoding = enc;
	m_length = length;
	m_stream = is;
}


void streamContentHandler::generate(utility::outputStream& os, const vmime::encoding& enc,
	const size_t maxLineLength) const
{
	if (!m_stream)
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
			shared_ptr <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();
			shared_ptr <utility::encoder::encoder> theEncoder = enc.getEncoder();

			theEncoder->getProperties()["maxlinelength"] = maxLineLength;
			theEncoder->getProperties()["text"] = (m_contentType.getType() == mediaTypes::TEXT);

			m_stream->reset();  // may not work...

			std::ostringstream oss;
			utility::outputStreamAdapter tempOut(oss);

			theDecoder->decode(*m_stream, tempOut);

			string str = oss.str();
			utility::inputStreamStringAdapter tempIn(str);

			theEncoder->encode(tempIn, os);
		}
		// No encoding to perform
		else
		{
			m_stream->reset();  // may not work...

			utility::bufferedStreamCopy(*m_stream, os);
		}
	}
	// Need to encode data before
	else
	{
		shared_ptr <utility::encoder::encoder> theEncoder = enc.getEncoder();
		theEncoder->getProperties()["maxlinelength"] = maxLineLength;
		theEncoder->getProperties()["text"] = (m_contentType.getType() == mediaTypes::TEXT);

		m_stream->reset();  // may not work...

		theEncoder->encode(*m_stream, os);
	}
}


void streamContentHandler::extract(utility::outputStream& os,
	utility::progressListener* progress) const
{
	if (!m_stream)
		return;

	// No decoding to perform
	if (!isEncoded())
	{
		m_stream->reset();  // may not work...

		if (progress)
			utility::bufferedStreamCopy(*m_stream, os, getLength(), progress);
		else
			utility::bufferedStreamCopy(*m_stream, os);
	}
	// Need to decode data
	else
	{
		shared_ptr <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();

		m_stream->reset();  // may not work...

		utility::progressListenerSizeAdapter plsa(progress, getLength());

		theDecoder->decode(*m_stream, os, &plsa);
	}
}


void streamContentHandler::extractRaw(utility::outputStream& os,
	utility::progressListener* progress) const
{
	if (!m_stream)
		return;

	m_stream->reset();  // may not work...

	if (progress)
		utility::bufferedStreamCopy(*m_stream, os, getLength(), progress);
	else
		utility::bufferedStreamCopy(*m_stream, os);
}


size_t streamContentHandler::getLength() const
{
	return (m_length);
}


bool streamContentHandler::isEmpty() const
{
	return (m_length == 0 || !m_stream);
}


bool streamContentHandler::isEncoded() const
{
	return (m_encoding != NO_ENCODING);
}


const vmime::encoding& streamContentHandler::getEncoding() const
{
	return (m_encoding);
}


bool streamContentHandler::isBuffered() const
{
	if (dynamicCast <utility::seekableInputStream>(m_stream) != NULL)
		return true;

	// FIXME: some streams can be resetted
	return false;
}


void streamContentHandler::setContentTypeHint(const mediaType& type)
{
	m_contentType = type;
}


const mediaType streamContentHandler::getContentTypeHint() const
{
	return m_contentType;
}


} // vmime
