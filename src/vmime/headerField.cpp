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

#include "vmime/headerField.hpp"
#include "vmime/headerFieldFactory.hpp"

#include "vmime/parserHelpers.hpp"

#include "vmime/exception.hpp"


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


shared_ptr <component> headerField::clone() const
{
	shared_ptr <headerField> field = headerFieldFactory::getInstance()->create(m_name);

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


shared_ptr <headerField> headerField::parseNext
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	size_t pos = position;

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

			return null;
		}
		else if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
		{
			if (newPosition)
				*newPosition = pos + 2;   // CR+LF

			return null;
		}

		// This line may be a field description
		if (!parserHelpers::isSpace(c))
		{
			const size_t nameStart = pos;  // remember the start position of the line

			while (pos < end && (buffer[pos] != ':' && !parserHelpers::isSpace(buffer[pos])))
				++pos;

			const size_t nameEnd = pos;

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

				const size_t contentsStart = pos;
				size_t contentsEnd = 0;

				bool firstLine = true;

				// Parse field value, taking care of line folding (value on multiple lines)
				for (size_t eol = 0 ; parserHelpers::findEOL(buffer, pos, end, &eol) ; pos = eol)
				{
					// If the line does not start with a folding indicator (SPACE or TAB),
					// and this is not the first line, then stop parsing lines
					if (!firstLine && !(buffer[pos] == ' ' || buffer[pos] == '\t'))
						break;

					contentsEnd = eol;
					firstLine = false;
				}

				if (pos == end && contentsEnd == 0)
				{
					// End of data, and no CRLF was found at the end
					contentsEnd = end;
				}

				// Strip spaces from end of header lines
				while (contentsEnd > contentsStart &&
				       (buffer[contentsEnd - 1] == ' ' || buffer[contentsEnd - 1] == '\t' ||
				        buffer[contentsEnd - 1] == '\r' || buffer[contentsEnd - 1] == '\n'))
				{
					contentsEnd--;
				}

				// Return a new field
				shared_ptr <headerField> field = headerFieldFactory::getInstance()->create(name);

				field->parse(ctx, buffer, contentsStart, contentsEnd, NULL);
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

				return null;
			}
			else if (pos + 1 < end && buffer[pos] == '\r' && buffer[pos + 1] == '\n')
			{
				if (newPosition)
					*newPosition = pos + 2;   // CR+LF

				return null;
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

	return null;
}


void headerField::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	m_value->parse(ctx, buffer, position, end, newPosition);
}


void headerField::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	os << m_name + ": ";

	m_value->generate(ctx, os, curLinePos + m_name.length() + 2, newLinePos);
}


size_t headerField::getGeneratedSize(const generationContext& ctx)
{
	return m_name.length() + 2 /* ": " */ + m_value->getGeneratedSize(ctx);
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


const std::vector <shared_ptr <component> > headerField::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	if (m_value)
		list.push_back(m_value);

	return (list);
}


shared_ptr <const headerFieldValue> headerField::getValue() const
{
	return m_value;
}


shared_ptr <headerFieldValue> headerField::getValue()
{
	return m_value;
}


void headerField::setValue(shared_ptr <headerFieldValue> value)
{
	if (!headerFieldFactory::getInstance()->isValueTypeValid(*this, *value))
		throw exceptions::bad_field_value_type(getName());

	if (value != NULL)
		m_value = value;
}


void headerField::setValueConst(shared_ptr <const headerFieldValue> value)
{
	if (!headerFieldFactory::getInstance()->isValueTypeValid(*this, *value))
		throw exceptions::bad_field_value_type(getName());

	m_value = vmime::clone(value);
}


void headerField::setValue(const headerFieldValue& value)
{
	if (!headerFieldFactory::getInstance()->isValueTypeValid(*this, value))
		throw exceptions::bad_field_value_type(getName());

	m_value = vmime::clone(value);
}


void headerField::setValue(const string& value)
{
	parse(value);
}


} // vmime
