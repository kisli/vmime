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

#include "vmime/parameter.hpp"
#include "vmime/parameterFactory.hpp"


namespace vmime
{


parameter* parameter::clone() const
{
	parameter* p = parameterFactory::getInstance()->create(m_name);
	p->copyFrom(*this);

	return (p);
}


void parameter::copyFrom(const component& other)
{
	const parameter& param = dynamic_cast <const parameter&>(other);

	m_name = param.m_name;

	getValue().copyFrom(param.getValue());
}


parameter& parameter::operator=(const parameter& other)
{
	copyFrom(other);
	return (*this);
}


const string& parameter::getName() const
{
	return (m_name);
}


void parameter::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	getValue().parse(buffer, position, end, newPosition);

	setParsedBounds(position, end);
}


void parameter::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	if (pos + m_name.length() + 10 > maxLineLength)
	{
		os << NEW_LINE_SEQUENCE;
		pos = NEW_LINE_SEQUENCE_LENGTH;
	}

	os << m_name << "=";
	pos += m_name.length() + 1;

	generateValue(os, maxLineLength, pos, newLinePos);
}


void parameter::generateValue(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	std::ostringstream valueStream;
	utility::outputStreamAdapter valueStreamV(valueStream);

	// TODO: can we imagine having values that span on multiple lines?
	getValue().generate(valueStreamV, lineLengthLimits::infinite, 0, NULL);

	const string value(valueStream.str());

	std::ostringstream ss;
	string::const_iterator start = value.begin();
	bool quoted = false;

	for (string::const_iterator i = value.begin() ; i != value.end() ; ++i)
	{
		switch (*i)
		{
		// Characters that need to be quoted _and_ escaped
		case '"':
		case '\\':

			ss << string(start, i) << "\\" << *i;

			start = i + 1;
			quoted = true;

			break;

		// Other characters that need quoting
		case ' ':
		case '\t':
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':

			quoted = true;
			break;
		}
	}

	if (start != value.end())
		ss << string(start, value.end());

	if (quoted)
		os << "\"" << ss.str() << "\"";
	else
		os << ss.str();

	if (newLinePos)
		*newLinePos = curLinePos + ss.str().length() + 2;
}


const std::vector <const component*> parameter::getChildComponents() const
{
	std::vector <const component*> list;

	list.push_back(&getValue());

	return (list);
}


} // vmime
