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

#include "bodyPart.hpp"


namespace vmime
{


bodyPart::bodyPart()
	: m_body(this), m_parent(NULL)
{
}


void bodyPart::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	// Parse the headers
	string::size_type pos = position;
	m_header.parse(buffer, pos, end, &pos);

	// Parse the body contents
	m_body.parse(buffer, pos, end, NULL);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void bodyPart::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	m_header.generate(os, maxLineLength);

	os << CRLF;

	m_body.generate(os, maxLineLength);

	if (newLinePos)
		*newLinePos = 0;
}


bodyPart* bodyPart::clone() const
{
	bodyPart* p = new bodyPart;

	p->m_parent = NULL;

	p->m_header.copyFrom(m_header);
	p->m_body.copyFrom(m_body);

	return (p);
}


void bodyPart::copyFrom(const component& other)
{
	const bodyPart& bp = dynamic_cast <const bodyPart&>(other);

	m_header = bp.m_header;
	m_body = bp.m_body;
}


bodyPart& bodyPart::operator=(const bodyPart& other)
{
	copyFrom(other);
	return (*this);
}


const header* bodyPart::getHeader() const
{
	return (&m_header);
}


header* bodyPart::getHeader()
{
	return (&m_header);
}


const body* bodyPart::getBody() const
{
	return (&m_body);
}


body* bodyPart::getBody()
{
	return (&m_body);
}


bodyPart* bodyPart::getParentPart() const
{
	return (m_parent);
}


const std::vector <const component*> bodyPart::getChildComponents() const
{
	std::vector <const component*> list;

	list.push_back(&m_header);
	list.push_back(&m_body);

	return (list);
}


} // vmime

