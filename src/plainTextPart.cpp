//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/plainTextPart.hpp"
#include "vmime/header.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


const mediaType plainTextPart::getType() const
{
	return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
}


const int plainTextPart::getPartCount() const
{
	return (1);
}


void plainTextPart::generateIn(bodyPart& /* message */, bodyPart& parent) const
{
	// Create a new part
	bodyPart* part = new bodyPart();
	parent.getBody()->appendPart(part);

	// Set header fields
	part->getHeader()->ContentType().setValue(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
	part->getHeader()->ContentType().setCharset(m_charset);
	part->getHeader()->ContentTransferEncoding().setValue(encoding(encodingTypes::QUOTED_PRINTABLE));

	// Set contents
	part->getBody()->setContents(m_text);
}


void plainTextPart::parse(const bodyPart& /* message */,
	const bodyPart& /* parent */, const bodyPart& textPart)
{
	m_text = textPart.getBody()->getContents();

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*textPart.getHeader()->findField(fields::CONTENT_TYPE));

		m_charset = ctf.getCharset();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field.
	}
	catch (exceptions::no_such_parameter)
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


const contentHandler& plainTextPart::getText() const
{
	return (m_text);
}


void plainTextPart::setText(const contentHandler& text)
{
	m_text = text;
}


} // vmime
