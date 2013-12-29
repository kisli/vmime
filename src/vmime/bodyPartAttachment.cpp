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

#include "vmime/bodyPartAttachment.hpp"


namespace vmime
{


bodyPartAttachment::bodyPartAttachment(shared_ptr <const bodyPart> part)
	: m_part(part)
{
}


const mediaType bodyPartAttachment::getType() const
{
	shared_ptr <const contentTypeField> ctf = getContentType();

	if (ctf)
	{
		return *ctf->getValue <mediaType>();
	}
	else
	{
		// No "Content-type" field: assume "application/octet-stream".
		return mediaType(mediaTypes::APPLICATION,
		                 mediaTypes::APPLICATION_OCTET_STREAM);
	}
}


const word bodyPartAttachment::getName() const
{
	word name;

	// Try the 'filename' parameter of 'Content-Disposition' field
	shared_ptr <const contentDispositionField> cdf = getContentDisposition();

	if (cdf && cdf->hasFilename())
	{
		name = cdf->getFilename();
	}
	// Try the 'name' parameter of 'Content-Type' field
	else
	{
		shared_ptr <const contentTypeField> ctf = getContentType();

		if (ctf)
		{
			shared_ptr <const parameter> prm = ctf->findParameter("name");

			if (prm != NULL)
				name = prm->getValue();
		}
	}

	return name;
}


const text bodyPartAttachment::getDescription() const
{
	text description;

	shared_ptr <const headerField> cd =
		getHeader()->findField(fields::CONTENT_DESCRIPTION);

	if (cd)
	{
		description = *cd->getValue <text>();
	}
	else
	{
		// No description available.
	}

	return description;
}


const encoding bodyPartAttachment::getEncoding() const
{
	return m_part->getBody()->getEncoding();
}


const shared_ptr <const contentHandler> bodyPartAttachment::getData() const
{
	return m_part->getBody()->getContents();
}


shared_ptr <const object> bodyPartAttachment::getPart() const
{
	return m_part;
}


shared_ptr <const header> bodyPartAttachment::getHeader() const
{
	return m_part->getHeader();
}


shared_ptr <const contentDispositionField> bodyPartAttachment::getContentDisposition() const
{
	return getHeader()->findField <contentDispositionField>(fields::CONTENT_DISPOSITION);
}


shared_ptr <const contentTypeField> bodyPartAttachment::getContentType() const
{
	return getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);
}


void bodyPartAttachment::generateIn(shared_ptr <bodyPart> /* parent */) const
{
	// Not used
}


} // vmime

