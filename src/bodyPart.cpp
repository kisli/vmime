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

#include "vmime/bodyPart.hpp"


namespace vmime
{


bodyPart::bodyPart()
	: m_header(make_shared <header>()),
	  m_body(make_shared <body>()),
	  m_parent()
{
	m_body->setParentPart(this);
}


void bodyPart::parseImpl
	(const parsingContext& ctx, shared_ptr <utility::parserInputStreamAdapter> parser,
	 const size_t position, const size_t end, size_t* newPosition)
{
	// Parse the headers
	size_t pos = position;
	m_header->parse(ctx, parser, pos, end, &pos);

	// Parse the body contents
	m_body->parse(ctx, parser, pos, end, NULL);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void bodyPart::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t /* curLinePos */, size_t* newLinePos) const
{
	m_header->generate(ctx, os);

	os << CRLF;

	m_body->generate(ctx, os);

	if (newLinePos)
		*newLinePos = 0;
}


size_t bodyPart::getGeneratedSize(const generationContext& ctx)
{
	return m_header->getGeneratedSize(ctx) + 2 /* CRLF */ + m_body->getGeneratedSize(ctx);
}


shared_ptr <component> bodyPart::clone() const
{
	shared_ptr <bodyPart> p = make_shared <bodyPart>();

	p->m_parent = NULL;

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


const shared_ptr <const header> bodyPart::getHeader() const
{
	return (m_header);
}


shared_ptr <header> bodyPart::getHeader()
{
	return (m_header);
}


void bodyPart::setHeader(shared_ptr <header> h)
{
	m_header = h;
}


const shared_ptr <const body> bodyPart::getBody() const
{
	return (m_body);
}


shared_ptr <body> bodyPart::getBody()
{
	return (m_body);
}


void bodyPart::setBody(shared_ptr <body> b)
{
	bodyPart* oldPart = b->m_part;

	m_body = b;
	m_body->setParentPart(this);

	// A body is associated to one and only one part
	if (oldPart != NULL)
		oldPart->setBody(make_shared <body>());
}


bodyPart* bodyPart::getParentPart()
{
	return m_parent;
}


const bodyPart* bodyPart::getParentPart() const
{
	return m_parent;
}


shared_ptr <bodyPart> bodyPart::createChildPart()
{
	shared_ptr <bodyPart> part = make_shared <bodyPart>();
	part->m_parent = this;

	return part;
}


void bodyPart::importChildPart(shared_ptr <bodyPart> part)
{
	part->m_parent = this;
}


const std::vector <shared_ptr <component> > bodyPart::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	list.push_back(m_header);
	list.push_back(m_body);

	return (list);
}


} // vmime

