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

	m_value->copyFrom(*hf.m_value);
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

					// Check for folded line
					if (c == '\r' && pos + 2 < end && buffer[pos + 1] == '\n' &&
						(buffer[pos + 2] == ' ' || buffer[pos + 2] == '\t'))
					{
						pos += 3;
					}
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

						// If the line contains only space characters, we assume it is
						// the end of the headers. This is not strictly standard-compliant
						// but, hey, we can't fail when parsing some malformed mails...
						while (pos < end && (buffer[pos] == ' ' || buffer[pos] == '\t'))
							++pos;

						if ((pos < end && buffer[pos] == '\n') ||
						    (pos + 1 < end && buffer[pos] == '\r' && buffer[pos + 1] == '\n'))
						{
							break;
						}
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
			// If the line contains only space characters, we assume it is
			// the end of the headers.
			while (pos < end && (buffer[pos] == ' ' || buffer[pos] == '\t'))
				++pos;

			if (pos < end && buffer[pos] == '\n')
			{
				if (newPosition)
					*newPosition = pos + 1;   // LF: illegal

				return NULL;
			}
			else if (pos + 1 < end && buffer[pos] == '\r' && buffer[pos + 1] == '\n')
			{
				if (newPosition)
					*newPosition = pos + 2;   // CR+LF

				return NULL;
			}

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


void headerField::parseImpl(const string& buffer, const string::size_type position, const string::size_type end,
	string::size_type* newPosition)
{
	m_value->parse(buffer, position, end, newPosition);
}


void headerField::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name + ": ";

	m_value->generate(os, maxLineLength, curLinePos + m_name.length() + 2, newLinePos);
}


const string headerField::getName() const
{
	return m_name;
}


void headerField::setName(const string& name)
{
	m_name = name;
}


bool headerField::isCustom() const
{
	return (m_name.length() > 2 && m_name[0] == 'X' && m_name[1] == '-');
}


const std::vector <ref <component> > headerField::getChildComponents()
{
	std::vector <ref <component> > list;

	if (m_value)
		list.push_back(m_value);

	return (list);
}


ref <const headerFieldValue> headerField::getValue() const
{
	return m_value;
}


ref <headerFieldValue> headerField::getValue()
{
	return m_value;
}


void headerField::setValue(ref <headerFieldValue> value)
{
	if (value != NULL)
		m_value = value;
}


void headerField::setValueConst(ref <const headerFieldValue> value)
{
	m_value = value->clone().dynamicCast <headerFieldValue>();
}


void headerField::setValue(const headerFieldValue& value)
{
	m_value = value.clone().dynamicCast <headerFieldValue>();
}


void headerField::setValue(const string& value)
{
	parse(value);
}


} // vmime
