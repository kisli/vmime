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

#include "headerField.hpp"
#include "headerFieldFactory.hpp"


namespace vmime
{


headerField::headerField()
	: m_name("X-Undefined")
{
}


headerField::headerField(const string& fieldName)
	: m_name(fieldName)
{
}


headerField::~headerField()
{
}


headerField* headerField::clone() const
{
	headerField* field = headerFieldFactory::getInstance()->create(m_name);

	field->copyFrom(*this);

	return (field);
}


void headerField::copyFrom(const component& other)
{
	const headerField& hf = dynamic_cast <const headerField&>(other);

	getValue().copyFrom(hf.getValue());
}


headerField& headerField::operator=(const headerField& other)
{
	copyFrom(other);
	return (*this);
}


void headerField::parse(const string& buffer, const string::size_type position, const string::size_type end,
	string::size_type* newPosition)
{
	getValue().parse(buffer, position, end, newPosition);

	setParsedBounds(position, end);
}


void headerField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name + ": ";

	getValue().generate(os, maxLineLength, curLinePos + m_name.length() + 2, newLinePos);
}


const string headerField::getName() const
{
	return (m_name);
}


const bool headerField::isCustom() const
{
	return (m_name.length() > 2 && m_name[0] == 'X' && m_name[1] == '-');
}


const std::vector <const component*> headerField::getChildComponents() const
{
	std::vector <const component*> list;

	list.push_back(&getValue());

	return (list);
}


} // vmime
