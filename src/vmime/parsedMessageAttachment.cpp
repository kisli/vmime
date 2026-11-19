//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include "vmime/parsedMessageAttachment.hpp"

#include "vmime/stringContentHandler.hpp"
#include "vmime/contentDisposition.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/details/classicOStringStream.hpp"


namespace vmime {


parsedMessageAttachment::parsedMessageAttachment(const shared_ptr <message>& msg)
	: m_msg(msg) {

}


mediaType parsedMessageAttachment::getType() const {

	return mediaType(mediaTypes::MESSAGE, mediaTypes::MESSAGE_RFC822);
}


text parsedMessageAttachment::getDescription() const {

	return text();
}


word parsedMessageAttachment::getName() const {

	return word();
}


shared_ptr <const contentHandler> parsedMessageAttachment::getData() const {

	if (!m_data) {

		utility::details::classicOStringStream oss;
		utility::outputStreamAdapter os(oss);

		m_msg->generate(os);

		const string data = oss.str();

		m_data = make_shared <stringContentHandler>(data);
		m_dataType = getDataType(data);
	}

	return m_data;
}


// static
const char* parsedMessageAttachment::getDataType(const string& data) {

	// 7bit and 8bit data have lines of 998 octets or less between CRLF, and
	// no NUL; 7bit data has no octets greater than 127; binary data may have
	// any sequence of octets (RFC 2045, sections 2.7 to 2.9)
	bool eightBit = false;
	size_t lineLength = 0;

	for (size_t i = 0, n = data.length() ; i < n ; ++i) {

		const unsigned char c = data[i];

		if (c == '\r' && i + 1 < n && data[i + 1] == '\n') {

			lineLength = 0;
			++i;

			continue;
		}

		// NUL, CR or LF alone, or a line which is too long
		if (c == '\0' || c == '\r' || c == '\n' || ++lineLength > 998) {
			return encodingTypes::BINARY;
		}

		if (c >= 128) {
			eightBit = true;
		}
	}

	return eightBit ? encodingTypes::EIGHT_BIT : encodingTypes::SEVEN_BIT;
}


encoding parsedMessageAttachment::getEncoding() const {

	// The message cannot be encoded: the encoding of a message/rfc822
	// part only tells whether it contains 8-bit or binary data (RFC 2046,
	// section 5.2.1)
	getData();

	return encoding(m_dataType);
}


shared_ptr <const object> parsedMessageAttachment::getPart() const {

	return null;
}


shared_ptr <const header> parsedMessageAttachment::getHeader() const {

	return null;
}


shared_ptr <message> parsedMessageAttachment::getMessage() const {

	return m_msg;
}


void parsedMessageAttachment::generateIn(const shared_ptr <bodyPart>& parent) const {

	// Create and append a new part for this attachment
	shared_ptr <bodyPart> part = make_shared <bodyPart>();
	parent->getBody()->appendPart(part);

	// Set header fields
	part->getHeader()->ContentType()->setValue(getType());
	part->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::ATTACHMENT));

	// Set contents
	part->getBody()->setContents(getData());
	part->getBody()->setEncoding(getEncoding());
}


} // vmime
