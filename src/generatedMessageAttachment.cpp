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

#include "vmime/generatedMessageAttachment.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime
{


generatedMessageAttachment::generatedMessageAttachment(shared_ptr <const bodyPart> part)
	: m_bpa(make_shared <bodyPartAttachment>(part))
{
}


const mediaType generatedMessageAttachment::getType() const
{
	return mediaType(mediaTypes::MESSAGE, mediaTypes::MESSAGE_RFC822);
}


const text generatedMessageAttachment::getDescription() const
{
	return m_bpa->getDescription();
}


const word generatedMessageAttachment::getName() const
{
	return m_bpa->getName();
}


const shared_ptr <const contentHandler> generatedMessageAttachment::getData() const
{
	return m_bpa->getData();
}


const encoding generatedMessageAttachment::getEncoding() const
{
	return m_bpa->getEncoding();
}


shared_ptr <const object> generatedMessageAttachment::getPart() const
{
	return m_bpa->getPart();
}


shared_ptr <const header> generatedMessageAttachment::getHeader() const
{
	return m_bpa->getHeader();
}


shared_ptr <message> generatedMessageAttachment::getMessage() const
{
	if (m_msg == NULL)
	{
		// Extract data
		std::ostringstream oss;
		utility::outputStreamAdapter os(oss);

		getData()->extract(os);

		// Parse message
		m_msg = make_shared <message>();
		m_msg->parse(oss.str());
	}

	return m_msg;
}


void generatedMessageAttachment::generateIn(shared_ptr <bodyPart> /* parent */) const
{
	// Not used (see 'parsedMessageAttachment')
}


} // vmime

