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

#include "defaultParameter.hpp"


namespace vmime
{


void defaultParameter::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	parseValue(buffer, position, end);

	if (newPosition)
		*newPosition = end;
}


void defaultParameter::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	const string value = quotedValue();

	pos += m_name.length() + value.length() + 2;

	if (pos > maxLineLength)
	{
		os << NEW_LINE_SEQUENCE;
		pos = NEW_LINE_SEQUENCE_LENGTH;
	}

	os << m_name << "=" << value;

	if (newLinePos)
		*newLinePos = pos;
}


const string defaultParameter::quotedValue() const
{
	const string value(generateValue());

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

	return (quoted ? ("\"" + ss.str() + "\"") : (ss.str()));
}


void defaultParameter::copyFrom(const parameter& param)
{
	parameter::copyFrom(param);
}


} // vmime
