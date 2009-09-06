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

#include "vmime/bodyPartAttachment.hpp"


namespace vmime
{


bodyPartAttachment::bodyPartAttachment(ref <const bodyPart> part)
	: m_part(part)
{
}


const mediaType bodyPartAttachment::getType() const
{
	mediaType type;

	try
	{
		type = *getContentType()->getValue().dynamicCast <const mediaType>();
	}
	catch (exceptions::no_such_field&)
	{
		// No "Content-type" field: assume "application/octet-stream".
		type = mediaType(mediaTypes::APPLICATION,
				 mediaTypes::APPLICATION_OCTET_STREAM);
	}

	return type;
}


const word bodyPartAttachment::getName() const
{
	word name;

	// Try the 'filename' parameter of 'Content-Disposition' field
	try
	{
		name = getContentDisposition()->getFilename();
	}
	catch (exceptions::no_such_field&)
	{
		// No 'Content-Disposition' field
	}
	catch (exceptions::no_such_parameter&)
	{
		// No 'filename' parameter
	}

	// Try the 'name' parameter of 'Content-Type' field
	if (name.getBuffer().empty())
	{
		try
		{
			ref <parameter> prm = getContentType()->findParameter("name");

			if (prm != NULL)
				name = prm->getValue();
		}
		catch (exceptions::no_such_field&)
		{
			// No 'Content-Type' field
		}
		catch (exceptions::no_such_parameter&)
		{
			// No attachment name available
		}
	}

	return name;
}


const text bodyPartAttachment::getDescription() const
{
	text description;

	try
	{
		ref <const headerField> cd =
			getHeader()->findField(fields::CONTENT_DESCRIPTION);

		description = *cd->getValue().dynamicCast <const text>();
	}
	catch (exceptions::no_such_field&)
	{
		// No description available.
	}

	return description;
}


const encoding bodyPartAttachment::getEncoding() const
{
	return m_part->getBody()->getEncoding();
}


const ref <const contentHandler> bodyPartAttachment::getData() const
{
	return m_part->getBody()->getContents();
}


ref <const object> bodyPartAttachment::getPart() const
{
	return m_part;
}


ref <const header> bodyPartAttachment::getHeader() const
{
	return m_part->getHeader();
}


ref <const contentDispositionField> bodyPartAttachment::getContentDisposition() const
{
	return getHeader()->findField(fields::CONTENT_DISPOSITION).
		dynamicCast <const contentDispositionField>();
}


ref <const contentTypeField> bodyPartAttachment::getContentType() const
{
	return getHeader()->findField(fields::CONTENT_TYPE).
		dynamicCast <const contentTypeField>();
}


void bodyPartAttachment::generateIn(ref <bodyPart> /* parent */) const
{
	// Not used
}


} // vmime

