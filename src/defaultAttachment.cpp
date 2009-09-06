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

#include "vmime/defaultAttachment.hpp"

#include "vmime/contentDisposition.hpp"
#include "vmime/contentDispositionField.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


defaultAttachment::defaultAttachment()
{
}


defaultAttachment::defaultAttachment(ref <const contentHandler> data,
	const encoding& enc, const mediaType& type, const text& desc, const word& name)
	: m_type(type), m_desc(desc), m_data(data), m_encoding(enc), m_name(name)
{
}


defaultAttachment::defaultAttachment(ref <const contentHandler> data,
	const mediaType& type, const text& desc, const word& name)
	: m_type(type), m_desc(desc), m_data(data),
	  m_encoding(encoding::decide(data)), m_name(name)
{
}


defaultAttachment::defaultAttachment(const defaultAttachment& attach)
	: attachment(), m_type(attach.m_type), m_desc(attach.m_desc),
	  m_data(attach.m_data->clone().dynamicCast <contentHandler>()),
	  m_encoding(attach.m_encoding), m_name(attach.m_name)
{
}


defaultAttachment::~defaultAttachment()
{
}


defaultAttachment& defaultAttachment::operator=(const defaultAttachment& attach)
{
	m_type = attach.m_type;
	m_desc = attach.m_desc;
	m_name = attach.m_name;
	m_data = attach.m_data->clone().dynamicCast <contentHandler>();
	m_encoding = attach.m_encoding;

	return (*this);
}


void defaultAttachment::generateIn(ref <bodyPart> parent) const
{
	// Create and append a new part for this attachment
	ref <bodyPart> part = vmime::create <bodyPart>();
	parent->getBody()->appendPart(part);

	generatePart(part);
}


void defaultAttachment::generatePart(ref <bodyPart> part) const
{
	// Set header fields
	part->getHeader()->ContentType()->setValue(m_type);
	if (!m_desc.isEmpty()) part->getHeader()->ContentDescription()->setValue(m_desc);
	part->getHeader()->ContentTransferEncoding()->setValue(m_encoding);
	part->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::ATTACHMENT));
	part->getHeader()->ContentDisposition().dynamicCast <contentDispositionField>()->setFilename(m_name);

	// Set contents
	part->getBody()->setContents(m_data);
}


const mediaType defaultAttachment::getType() const
{
	return m_type;
}


const text defaultAttachment::getDescription() const
{
	return m_desc;
}


const word defaultAttachment::getName() const
{
	return m_name;
}


const ref <const contentHandler> defaultAttachment::getData() const
{
	return m_data;
}


const encoding defaultAttachment::getEncoding() const
{
	return m_encoding;
}


ref <const object> defaultAttachment::getPart() const
{
	return NULL;
}


ref <const header> defaultAttachment::getHeader() const
{
	return NULL;
}


} // vmime
