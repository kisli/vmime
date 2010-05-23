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

#include "vmime/plainTextPart.hpp"
#include "vmime/header.hpp"
#include "vmime/exception.hpp"

#include "vmime/contentTypeField.hpp"

#include "vmime/emptyContentHandler.hpp"


namespace vmime
{


plainTextPart::plainTextPart()
	: m_text(vmime::create <emptyContentHandler>())
{
}


plainTextPart::~plainTextPart()
{
}


const mediaType plainTextPart::getType() const
{
	return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
}


int plainTextPart::getPartCount() const
{
	return (1);
}


void plainTextPart::generateIn(ref <bodyPart> /* message */, ref <bodyPart> parent) const
{
	// Create a new part
	ref <bodyPart> part = vmime::create <bodyPart>();
	parent->getBody()->appendPart(part);

	// Set contents
	part->getBody()->setContents(m_text,
		mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN), m_charset,
		encoding::decide(m_text, m_charset, encoding::USAGE_TEXT));
}


void plainTextPart::parse(ref <const bodyPart> /* message */,
	ref <const bodyPart> /* parent */, ref <const bodyPart> textPart)
{
	m_text = textPart->getBody()->getContents()->clone().dynamicCast <contentHandler>();

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*textPart->getHeader()->findField(fields::CONTENT_TYPE));

		m_charset = ctf.getCharset();
	}
	catch (exceptions::no_such_field&)
	{
		// No "Content-type" field.
	}
	catch (exceptions::no_such_parameter&)
	{
		// No "charset" parameter.
	}
}


const charset& plainTextPart::getCharset() const
{
	return (m_charset);
}


void plainTextPart::setCharset(const charset& ch)
{
	m_charset = ch;
}


const ref <const contentHandler> plainTextPart::getText() const
{
	return (m_text);
}


void plainTextPart::setText(ref <contentHandler> text)
{
	m_text = text->clone().dynamicCast <contentHandler>();
}


} // vmime
