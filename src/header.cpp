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

#include "vmime/header.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


header::header()
{
}


header::~header()
{
	removeAllFields();
}


/*

 RFC #822:
 3.2.  HEADER FIELD DEFINITIONS

field       =  field-name ":" [ field-body ] CRLF

field-name  =  1*<any CHAR, excluding CTLs, SPACE, and ":">

field-body  =  field-body-contents
		[CRLF LWSP-char field-body]

field-body-contents =
		<the ASCII characters making up the field-body, as
		 defined in the following sections, and consisting
		 of combinations of atom, quoted-string, and
		 specials tokens, or else consisting of texts>
*/

void header::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	string::size_type pos = position;

	removeAllFields();

	while (pos < end)
	{
		char_t c = buffer[pos];

		// Check for end of headers (empty line): although RFC-822 recommends
		// to use CRLF for header/body separator (see 4.1 SYNTAX), here, we
		// also check for LF just in case...
		if (c == '\n')
		{
			++pos;
			break;
		}
		else if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
		{
			pos += 2;
			break;
		}

		// This line may be a field description
		if (!isspace(c))
		{
			const string::size_type nameStart = pos;  // remember the start position of the line

			while (pos < end && (buffer[pos] != ':' && !isspace(buffer[pos])))
				++pos;

			const string::size_type nameEnd = pos;

			while (pos < end && isspace(buffer[pos]))
				++pos;

			if (buffer[pos] != ':')
			{
				// Humm...does not seem to be a valid header line.
				// Skip this error and advance to the next line
				pos = nameStart;

				while (pos < end && buffer[pos] != '\n')
					++pos;

				if (buffer[pos] == '\n')
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

				// Extract the field value
				string contents;

				while (pos < end)
				{
					c = buffer[pos];

					// Check for end of contents
					if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
					{
						pos += 2;
						break;
					}
					else if (c == '\n')
					{
						++pos;
						break;
					}

					const string::size_type ctsStart = pos;
					string::size_type ctsEnd = pos;

					while (pos < end)
					{
						c = buffer[pos];

						// Check for end of line
						if (c == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
						{
							ctsEnd = pos;
							pos += 2;
							break;
						}
						else if (c == '\n')
						{
							ctsEnd = pos;
							++pos;
							break;
						}

						++pos;
					}

					if (ctsEnd != ctsStart)
					{
						// Append this line to contents
						contents.append(buffer.begin() + ctsStart,
						                buffer.begin() + ctsEnd);
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

				// Add a new field to list
				m_fields.push_back(headerFieldFactory::getInstance()->
					create(name, contents));
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

	// If we have found the header/body separator, skip it
	if (pos < end)
	{
		if (buffer[pos] == '\n')
		{
			// This is a LF (illegal but...)
			++pos;
		}
		else if (buffer[pos] == '\r' && pos + 1 < end)
		{
			// This is a CRLF
			pos += 2;
		}
	}

	setParsedBounds(position, pos);

	if (newPosition)
		*newPosition = pos;
}


void header::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	// Generate the fields
	for (std::vector <headerField*>::const_iterator it = m_fields.begin() ;
	     it != m_fields.end() ; ++it)
	{
		(*it)->generate(os, maxLineLength);
		os << CRLF;
	}

	if (newLinePos)
		*newLinePos = 0;
}


header* header::clone() const
{
	header* hdr = new header();

	try
	{
		hdr->m_fields.reserve(m_fields.size());

		for (std::vector <headerField*>::const_iterator it = m_fields.begin() ;
		     it != m_fields.end() ; ++it)
		{
			hdr->m_fields.push_back((*it)->clone());
		}
	}
	catch (std::exception&)
	{
		free_container(hdr->m_fields);

		delete (hdr);
		throw;
	}

	return (hdr);
}


void header::copyFrom(const component& other)
{
	const header& h = dynamic_cast <const header&>(other);

	std::vector <headerField*> fields;

	try
	{
		fields.reserve(h.m_fields.size());

		for (std::vector <headerField*>::const_iterator it = h.m_fields.begin() ;
		     it != h.m_fields.end() ; ++it)
		{
			fields.push_back((*it)->clone());
		}

		free_container(m_fields);

		m_fields.resize(fields.size());

		std::copy(fields.begin(), fields.end(), m_fields.begin());
	}
	catch (std::exception&)
	{
		free_container(fields);
		throw;
	}
}


header& header::operator=(const header& other)
{
	copyFrom(other);
	return (*this);
}


const bool header::hasField(const string& fieldName) const
{
	const string name = utility::stringUtils::toLower(fieldName);

	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	return (pos != end);
}


headerField* header::findField(const string& fieldName) const
{
	const string name = utility::stringUtils::toLower(fieldName);

	// Find the first field that matches the specified name
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	// No field with this name can be found
	if (pos == end)
	{
		throw exceptions::no_such_field();
	}
	// Else, return a reference to the existing field
	else
	{
		return (*pos);
	}
}


std::vector <headerField*> header::findAllFields(const string& fieldName)
{
	const string name = utility::stringUtils::toLower(fieldName);

	std::vector <headerField*> result;

	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end ; ++pos)
	{
		// Add the header if it matches the specified type
		if (utility::stringUtils::toLower((*pos)->getName()) == name)
		{
			result.push_back(*pos);
		}
	}

	return result;
}


headerField* header::getField(const string& fieldName)
{
	const string name = utility::stringUtils::toLower(fieldName);

	// Find the first field that matches the specified name
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	// If no field with this name can be found, create a new one
	if (pos == end)
	{
		headerField* field = headerFieldFactory::getInstance()->create(fieldName);

		try
		{
			appendField(field);
		}
		catch (std::exception&)
		{
			delete (field);
			throw;
		}

		// Return a reference to the new field
		return (field);
	}
	// Else, return a reference to the existing field
	else
	{
		return (*pos);
	}
}


void header::appendField(headerField* field)
{
	m_fields.push_back(field);
}


void header::insertFieldBefore(headerField* beforeField, headerField* field)
{
	const std::vector <headerField*>::iterator it = std::find
		(m_fields.begin(), m_fields.end(), beforeField);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	m_fields.insert(it, field);
}


void header::insertFieldBefore(const int pos, headerField* field)
{
	m_fields.insert(m_fields.begin() + pos, field);
}


void header::insertFieldAfter(headerField* afterField, headerField* field)
{
	const std::vector <headerField*>::iterator it = std::find
		(m_fields.begin(), m_fields.end(), afterField);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	m_fields.insert(it + 1, field);
}


void header::insertFieldAfter(const int pos, headerField* field)
{
	m_fields.insert(m_fields.begin() + pos + 1, field);
}


void header::removeField(headerField* field)
{
	const std::vector <headerField*>::iterator it = std::find
		(m_fields.begin(), m_fields.end(), field);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	delete (*it);

	m_fields.erase(it);
}


void header::removeField(const int pos)
{
	const std::vector <headerField*>::iterator it = m_fields.begin() + pos;

	delete (*it);

	m_fields.erase(it);
}


void header::removeAllFields()
{
	free_container(m_fields);
}


const int header::getFieldCount() const
{
	return (m_fields.size());
}


const bool header::isEmpty() const
{
	return (m_fields.empty());
}


headerField* header::getFieldAt(const int pos)
{
	return (m_fields[pos]);
}


const headerField* const header::getFieldAt(const int pos) const
{
	return (m_fields[pos]);
}


const std::vector <const headerField*> header::getFieldList() const
{
	std::vector <const headerField*> list;

	list.reserve(m_fields.size());

	for (std::vector <headerField*>::const_iterator it = m_fields.begin() ;
	     it != m_fields.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <headerField*> header::getFieldList()
{
	return (m_fields);
}


const std::vector <const component*> header::getChildComponents() const
{
	std::vector <const component*> list;

	copy_vector(m_fields, list);

	return (list);
}


} // vmime
