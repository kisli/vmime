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

#include "vmime/net/imap/IMAPMessagePartContentHandler.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringProxyAdapter.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPMessagePartContentHandler::IMAPMessagePartContentHandler
	(ref <IMAPMessage> msg, ref <class part> part, const vmime::encoding& encoding)
	: m_message(msg), m_part(part), m_encoding(encoding)
{
}


ref <contentHandler> IMAPMessagePartContentHandler::clone() const
{
	return create <IMAPMessagePartContentHandler>
		(m_message.acquire().constCast <IMAPMessage>(),
		 m_part.acquire().constCast <part>(),
		 m_encoding);
}


void IMAPMessagePartContentHandler::generate
	(utility::outputStream& os, const vmime::encoding& enc, const string::size_type maxLineLength) const
{
	ref <IMAPMessage> msg = m_message.acquire().constCast <IMAPMessage>();
	ref <part> part = m_part.acquire().constCast <class part>();

	// Data is already encoded
	if (isEncoded())
	{
		// The data is already encoded but the encoding specified for
		// the generation is different from the current one. We need
		// to re-encode data: decode from input buffer to temporary
		// buffer, and then re-encode to output stream...
		if (m_encoding != enc)
		{
			// Extract part contents to temporary buffer
			std::ostringstream oss;
			utility::outputStreamAdapter tmp(oss);

			msg->extractPart(part, tmp, NULL);

			// Decode to another temporary buffer
			utility::inputStreamStringProxyAdapter in(oss.str());

			std::ostringstream oss2;
			utility::outputStreamAdapter tmp2(oss2);

			ref <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();
			theDecoder->decode(in, tmp2);

			// Reencode to output stream
			string str = oss2.str();
			utility::inputStreamStringAdapter tempIn(str);

			ref <utility::encoder::encoder> theEncoder = enc.getEncoder();
			theEncoder->getProperties()["maxlinelength"] = maxLineLength;
			theEncoder->encode(tempIn, os);
		}
		// No encoding to perform
		else
		{
			msg->extractPart(part, os);
		}
	}
	// Need to encode data before
	else
	{
		// Extract part contents to temporary buffer
		std::ostringstream oss;
		utility::outputStreamAdapter tmp(oss);

		msg->extractPart(part, tmp, NULL);

		// Encode temporary buffer to output stream
		ref <utility::encoder::encoder> theEncoder = enc.getEncoder();
		theEncoder->getProperties()["maxlinelength"] = maxLineLength;

		utility::inputStreamStringAdapter is(oss.str());

		theEncoder->encode(is, os);
	}
}


void IMAPMessagePartContentHandler::extract
	(utility::outputStream& os, utility::progressListener* progress) const
{
	ref <IMAPMessage> msg = m_message.acquire().constCast <IMAPMessage>();
	ref <part> part = m_part.acquire().constCast <class part>();

	// No decoding to perform
	if (!isEncoded())
	{
		msg->extractPart(part, os, progress);
	}
	// Need to decode data
	else
	{
		// Extract part contents to temporary buffer
		std::ostringstream oss;
		utility::outputStreamAdapter tmp(oss);

		msg->extractPart(part, tmp, NULL);

		// Encode temporary buffer to output stream
		utility::inputStreamStringAdapter is(oss.str());
		utility::progressListenerSizeAdapter plsa(progress, getLength());

		ref <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();
		theDecoder->decode(is, os, &plsa);
	}
}


void IMAPMessagePartContentHandler::extractRaw
	(utility::outputStream& os, utility::progressListener* progress) const
{
	ref <IMAPMessage> msg = m_message.acquire().constCast <IMAPMessage>();
	ref <part> part = m_part.acquire().constCast <class part>();

	msg->extractPart(part, os, progress);
}


string::size_type IMAPMessagePartContentHandler::getLength() const
{
	return m_part.acquire()->getSize();
}


bool IMAPMessagePartContentHandler::isEncoded() const
{
	return m_encoding != NO_ENCODING;
}


const vmime::encoding& IMAPMessagePartContentHandler::getEncoding() const
{
	return m_encoding;
}


bool IMAPMessagePartContentHandler::isEmpty() const
{
	return getLength() == 0;
}


bool IMAPMessagePartContentHandler::isBuffered() const
{
	return true;
}


} // imap
} // net
} // vmime

