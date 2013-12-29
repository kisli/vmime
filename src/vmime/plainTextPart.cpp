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

#include "vmime/plainTextPart.hpp"
#include "vmime/header.hpp"
#include "vmime/exception.hpp"

#include "vmime/contentTypeField.hpp"

#include "vmime/emptyContentHandler.hpp"


namespace vmime
{


plainTextPart::plainTextPart()
	: m_text(make_shared <emptyContentHandler>())
{
}


plainTextPart::~plainTextPart()
{
}


const mediaType plainTextPart::getType() const
{
	return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
}


size_t plainTextPart::getPartCount() const
{
	return (1);
}


void plainTextPart::generateIn(shared_ptr <bodyPart> /* message */, shared_ptr <bodyPart> parent) const
{
	// Create a new part
	shared_ptr <bodyPart> part = make_shared <bodyPart>();
	parent->getBody()->appendPart(part);

	// Set contents
	part->getBody()->setContents(m_text,
		mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN), m_charset,
		encoding::decide(m_text, m_charset, encoding::USAGE_TEXT));
}


void plainTextPart::parse(shared_ptr <const bodyPart> /* message */,
	shared_ptr <const bodyPart> /* parent */, shared_ptr <const bodyPart> textPart)
{
	m_text = vmime::clone(textPart->getBody()->getContents());

	shared_ptr <const contentTypeField> ctf =
		textPart->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf && ctf->hasCharset())
		m_charset = ctf->getCharset();
	else
		m_charset = charset();
}


const charset& plainTextPart::getCharset() const
{
	return (m_charset);
}


void plainTextPart::setCharset(const charset& ch)
{
	m_charset = ch;
}


const shared_ptr <const contentHandler> plainTextPart::getText() const
{
	return (m_text);
}


void plainTextPart::setText(shared_ptr <contentHandler> text)
{
	m_text = vmime::clone(text);
}


} // vmime
