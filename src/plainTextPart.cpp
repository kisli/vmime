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

#include "plainTextPart.hpp"
#include "header.hpp"
#include "exception.hpp"


namespace vmime
{


const mediaType plainTextPart::type() const
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
	parent.body().parts.append(part);

	// Set header fields
	part->header().fields.ContentType() = mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN);
	part->header().fields.ContentType().charset() = m_charset;
	part->header().fields.ContentTransferEncoding() = encoding(encodingTypes::QUOTED_PRINTABLE);

	// Set contents
	part->body().contents() = m_text;
}


void plainTextPart::parse(const bodyPart& /* message */,
	const bodyPart& /* parent */, const bodyPart& textPart)
{
	m_text = textPart.body().contents();

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(textPart.header().fields.find(headerField::ContentType));

		m_charset = ctf.charset();
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


} // vmime
