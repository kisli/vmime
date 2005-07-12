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

#include "vmime/defaultAttachment.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


defaultAttachment::defaultAttachment()
{
}


defaultAttachment::defaultAttachment(ref <contentHandler> data,
	const encoding& enc, const mediaType& type, const text& desc)
	: m_type(type), m_desc(desc), m_data(data), m_encoding(enc)
{
}


defaultAttachment::defaultAttachment(ref <contentHandler> data,
	const mediaType& type, const text& desc)
	: m_type(type), m_desc(desc), m_data(data),
	  m_encoding(encoding::decide(data))
{
}


defaultAttachment::defaultAttachment(const defaultAttachment& attach)
	: attachment(), m_type(attach.m_type), m_desc(attach.m_desc),
	  m_data(attach.m_data->clone().dynamicCast <contentHandler>()), m_encoding(attach.m_encoding)
{
}


defaultAttachment::~defaultAttachment()
{
}


defaultAttachment& defaultAttachment::operator=(const defaultAttachment& attach)
{
	m_type = attach.m_type;
	m_desc = attach.m_desc;
	m_data = attach.m_data->clone().dynamicCast <contentHandler>();
	m_encoding = attach.m_encoding;

	return (*this);
}


void defaultAttachment::generateIn(bodyPart& parent) const
{
	// Create and append a new part for this attachment
	ref <bodyPart> part = vmime::create <bodyPart>();
	parent.getBody()->appendPart(part);

	generatePart(*part);
}


void defaultAttachment::generatePart(bodyPart& part) const
{
	// Set header fields
	part.getHeader()->ContentType()->setValue(m_type);
	if (!m_desc.isEmpty()) part.getHeader()->ContentDescription()->setValue(m_desc);
	part.getHeader()->ContentTransferEncoding()->setValue(m_encoding);
	part.getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::ATTACHMENT));

	// Set contents
	part.getBody()->setContents(m_data);
}


const mediaType& defaultAttachment::getType() const
{
	return (m_type);
}


const text& defaultAttachment::getDescription() const
{
	return (m_desc);
}


const ref <const contentHandler> defaultAttachment::getData() const
{
	return (m_data);
}


const encoding& defaultAttachment::getEncoding() const
{
	return (m_encoding);
}


} // vmime
