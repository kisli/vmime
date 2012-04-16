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

#include "vmime/bodyPart.hpp"


namespace vmime
{


bodyPart::bodyPart()
	: m_header(vmime::create <header>()),
	  m_body(vmime::create <body>()),
	  m_parent(NULL)
{
	m_body->setParentPart(thisRef().dynamicCast <bodyPart>());
}


bodyPart::bodyPart(weak_ref <vmime::bodyPart> parentPart)
	: m_header(vmime::create <header>()),
	  m_body(vmime::create <body>()),
	  m_parent(parentPart)
{
	m_body->setParentPart(thisRef().dynamicCast <bodyPart>());
}


void bodyPart::parseImpl
	(ref <utility::parserInputStreamAdapter> parser,
	 const utility::stream::size_type position,
	 const utility::stream::size_type end,
	 utility::stream::size_type* newPosition)
{
	// Parse the headers
	string::size_type pos = position;
	m_header->parse(parser, pos, end, &pos);

	// Parse the body contents
	m_body->parse(parser, pos, end, NULL);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void bodyPart::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	m_header->generate(os, maxLineLength);

	os << CRLF;

	m_body->generate(os, maxLineLength);

	if (newLinePos)
		*newLinePos = 0;
}


ref <component> bodyPart::clone() const
{
	ref <bodyPart> p = vmime::create <bodyPart>();

	p->m_parent = null;

	p->m_header->copyFrom(*m_header);
	p->m_body->copyFrom(*m_body);

	return (p);
}


void bodyPart::copyFrom(const component& other)
{
	const bodyPart& bp = dynamic_cast <const bodyPart&>(other);

	m_header->copyFrom(*(bp.m_header));
	m_body->copyFrom(*(bp.m_body));
}


bodyPart& bodyPart::operator=(const bodyPart& other)
{
	copyFrom(other);
	return (*this);
}


const ref <const header> bodyPart::getHeader() const
{
	return (m_header);
}


ref <header> bodyPart::getHeader()
{
	return (m_header);
}


const ref <const body> bodyPart::getBody() const
{
	return (m_body);
}


ref <body> bodyPart::getBody()
{
	return (m_body);
}


ref <bodyPart> bodyPart::getParentPart()
{
	return m_parent.acquire();
}


ref <const bodyPart> bodyPart::getParentPart() const
{
	return m_parent.acquire();
}


const std::vector <ref <component> > bodyPart::getChildComponents()
{
	std::vector <ref <component> > list;

	list.push_back(m_header);
	list.push_back(m_body);

	return (list);
}


} // vmime

