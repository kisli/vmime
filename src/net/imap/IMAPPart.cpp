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

#include "vmime/net/imap/IMAPPart.hpp"
#include "vmime/net/imap/IMAPStructure.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPPart::IMAPPart(ref <IMAPPart> parent, const int number, const IMAPParser::body_type_mpart* mpart)
	: m_parent(parent), m_header(NULL), m_number(number), m_size(0)
{
	m_mediaType = vmime::mediaType
		("multipart", mpart->media_subtype()->value());
}


IMAPPart::IMAPPart(ref <IMAPPart> parent, const int number, const IMAPParser::body_type_1part* part)
	: m_parent(parent), m_header(NULL), m_number(number), m_size(0)
{
	if (part->body_type_text())
	{
		m_mediaType = vmime::mediaType
			("text", part->body_type_text()->
				media_text()->media_subtype()->value());

		m_size = part->body_type_text()->body_fields()->body_fld_octets()->value();
	}
	else if (part->body_type_msg())
	{
		m_mediaType = vmime::mediaType
			("message", part->body_type_msg()->
				media_message()->media_subtype()->value());
	}
	else
	{
		m_mediaType = vmime::mediaType
			(part->body_type_basic()->media_basic()->media_type()->value(),
			 part->body_type_basic()->media_basic()->media_subtype()->value());

		m_size = part->body_type_basic()->body_fields()->body_fld_octets()->value();
	}

	m_structure = NULL;
}


ref <const structure> IMAPPart::getStructure() const
{
	if (m_structure != NULL)
		return m_structure;
	else
		return IMAPStructure::emptyStructure();
}


ref <structure> IMAPPart::getStructure()
{
	if (m_structure != NULL)
		return m_structure;
	else
		return IMAPStructure::emptyStructure();
}


ref <const IMAPPart> IMAPPart::getParent() const
{
	return m_parent.acquire();
}


const mediaType& IMAPPart::getType() const
{
	return m_mediaType;
}


int IMAPPart::getSize() const
{
	return m_size;
}


int IMAPPart::getNumber() const
{
	return m_number;
}


ref <const header> IMAPPart::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();
	else
		return m_header;
}


// static
ref <IMAPPart> IMAPPart::create
	(ref <IMAPPart> parent, const int number, const IMAPParser::body* body)
{
	if (body->body_type_mpart())
	{
		ref <IMAPPart> part = vmime::create <IMAPPart>(parent, number, body->body_type_mpart());
		part->m_structure = vmime::create <IMAPStructure>(part, body->body_type_mpart()->list());

		return part;
	}
	else
	{
		return vmime::create <IMAPPart>(parent, number, body->body_type_1part());
	}
}


header& IMAPPart::getOrCreateHeader()
{
	if (m_header != NULL)
		return *m_header;
	else
		return *(m_header = vmime::create <header>());
}


} // imap
} // net
} // vmime

