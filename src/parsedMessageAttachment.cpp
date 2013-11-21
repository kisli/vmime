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

#include "vmime/parsedMessageAttachment.hpp"

#include "vmime/stringContentHandler.hpp"
#include "vmime/contentDisposition.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime
{


parsedMessageAttachment::parsedMessageAttachment(shared_ptr <message> msg)
	: m_msg(msg)
{
}


const mediaType parsedMessageAttachment::getType() const
{
	return mediaType(mediaTypes::MESSAGE, mediaTypes::MESSAGE_RFC822);
}


const text parsedMessageAttachment::getDescription() const
{
	return text();
}


const word parsedMessageAttachment::getName() const
{
	return word();
}


const shared_ptr <const contentHandler> parsedMessageAttachment::getData() const
{
	if (m_data == NULL)
	{
		std::ostringstream oss;
		utility::outputStreamAdapter os(oss);

		m_msg->generate(os);

		m_data = make_shared <stringContentHandler>(oss.str());
	}

	return m_data;
}


const encoding parsedMessageAttachment::getEncoding() const
{
	return encoding(encodingTypes::EIGHT_BIT);  // not important
}


shared_ptr <const object> parsedMessageAttachment::getPart() const
{
	return null;
}


shared_ptr <const header> parsedMessageAttachment::getHeader() const
{
	return null;
}


shared_ptr <message> parsedMessageAttachment::getMessage() const
{
	return m_msg;
}


void parsedMessageAttachment::generateIn(shared_ptr <bodyPart> parent) const
{
	// Create and append a new part for this attachment
	shared_ptr <bodyPart> part = make_shared <bodyPart>();
	parent->getBody()->appendPart(part);

	// Set header fields
	part->getHeader()->ContentType()->setValue(getType());
	part->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::ATTACHMENT));

	// Set contents
	part->getBody()->setContents(getData());
}


} // vmime

