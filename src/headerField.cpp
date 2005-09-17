//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/headerField.hpp"
#include "vmime/headerFieldFactory.hpp"

#include "vmime/parserHelpers.hpp"


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


ref <component> headerField::clone() const
{
	ref <headerField> field = headerFieldFactory::getInstance()->create(m_name);

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


ref <headerField> headerField::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	string::size_type pos = position;

	while (pos < end)
	{
		char_t c = buffer[pos];

		// Check for end of headers (empty line): although RFC-822 recommends
		// to use CRLF for header/body separator (see 4.1 SYNTAX), here, we
		// also check for LF for compatibility with broken implementations...
		if (c == '\n')
		{
			if (newPosition)
				*newPosition = pos + 1;   // LF: illegal

			return (NULL);
		}
		else if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
		{
			if (newPosition)
				*newPosition = pos + 2;   // CR+LF

			return (NULL);
		}

		// This line may be a field description
		if (!parserHelpers::isSpace(c))
		{
			const string::size_type nameStart = pos;  // remember the start position of the line

			while (pos < end && (buffer[pos] != ':' && !parserHelpers::isSpace(buffer[pos])))
				++pos;

			const string::size_type nameEnd = pos;

			while (pos < end && (buffer[pos] == ' ' || buffer[pos] == '\t'))
				++pos;

			if (buffer[pos] != ':')
			{
				// Humm...does not seem to be a valid header line.
				// Skip this error and advance to the next line
				pos = nameStart;

				while (pos < end && buffer[pos] != '\n')
					++pos;

				if (pos < end && buffer[pos] == '\n')
					++pos;
			}
			else
			{
				// Extract the field name
				const string name(buffer.begin() + nameStart,
				                  buffer.begin() + nameEnd);

				// Skip ':' character
				++pos;

				// Skip spaces between ':' and the field contents
				while (pos < end && (buffer[pos] == ' ' || buffer[pos] == '\t'))
					++pos;

				const string::size_type contentsStart = pos;
				string::size_type contentsEnd = 0;

				// Extract the field value
				while (pos < end)
				{
					c = buffer[pos];

					// Check for end of contents
					if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
					{
						contentsEnd = pos;
						pos += 2;
						break;
					}
					else if (c == '\n')
					{
						contentsEnd = pos;
						++pos;
						break;
					}

					while (pos < end)
					{
						c = buffer[pos];

						// Check for end of line
						if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
						{
							contentsEnd = pos;
							pos += 2;
							break;
						}
						else if (c == '\n')
						{
							contentsEnd = pos;
							++pos;
							break;
						}

						++pos;
					}

					// Handle the case of folded lines
					if (buffer[pos] == ' ' || buffer[pos] == '\t')
					{
						// This is a folding white-space: we keep it as is and
						// we continue with contents parsing...
					}
					else
					{
						// End of this field
						break;
					}
				}

				// Return a new field
				ref <headerField> field = headerFieldFactory::getInstance()->create(name);

				field->parse(buffer, contentsStart, contentsEnd, NULL);
				field->setParsedBounds(nameStart, pos);

				if (newPosition)
					*newPosition = pos;

				return (field);
			}
		}
		else
		{
			// Skip this error and advance to the next line
			while (pos < end && buffer[pos] != '\n')
				++pos;

			if (buffer[pos] == '\n')
				++pos;
		}
	}

	if (newPosition)
		*newPosition = pos;

	return (NULL);
}


void headerField::parse(const string& buffer, const string::size_type position, const string::size_type end,
	string::size_type* newPosition)
{
	getValue().parse(buffer, position, end, newPosition);
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


const std::vector <ref <const component> > headerField::getChildComponents() const
{
	std::vector <ref <const component> > list;

	list.push_back(getValueImp());

	return (list);
}


void headerField::setValue(const string& value)
{
	parse(value);
}


} // vmime
