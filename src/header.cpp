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

#include "header.hpp"
#include "parserHelpers.hpp"


namespace vmime
{


header::header()
{
}


header::~header()
{
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

	fields.clear();

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
				fields.m_fields.push_back(headerFieldFactory::getInstance()->
					create(headerField::nameToType(name), name, contents));
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

	if (newPosition)
		*newPosition = pos;
}


void header::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	// Generate the fields
	for (std::vector <headerField*>::const_iterator
	     it = fields.m_fields.begin() ; it != fields.m_fields.end() ; ++it)
	{
		(*it)->generate(os, maxLineLength);
		os << CRLF;
	}

	if (newLinePos)
		*newLinePos = 0;
}


header& header::operator=(const header& h)
{
	fields = h.fields;

	return (*this);
}



//////////////////////
// Fields container //
//////////////////////


header::fieldsContainer::fieldsContainer()
{
}


header::fieldsContainer::~fieldsContainer()
{
	for (std::vector <headerField*>::iterator i = m_fields.begin() ; i != m_fields.end() ; ++i)
		delete (*i);
}


// Checks whether (at least) one field with this type/name exists
const bool header::fieldsContainer::has(const headerField::Types fieldType) const
{
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && (*pos)->type() != fieldType ; ++pos);

	return (pos != end);
}


const bool header::fieldsContainer::has(const string& fieldName) const
{
	headerField::Types type = headerField::nameToType(fieldName);
	if (type != headerField::Custom) return (has(type));

	const string name = toLower(fieldName);

	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && toLower((*pos)->name()) != name ; ++pos);

	return (pos != end);
}


// Find the first field that matches the specified type/name.
// If no field is found, an exception is thrown.
headerField& header::fieldsContainer::find(const headerField::Types fieldType) const
{
	// Find the first field that matches the specified type
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && (*pos)->type() != fieldType ; ++pos);

	// No field with this type can be found
	if (pos == end)
	{
		throw exceptions::no_such_field();
	}
	// Else, return a reference to the existing field
	else
	{
		return (**pos);
	}
}


headerField& header::fieldsContainer::find(const string& fieldName) const
{
	headerField::Types type = headerField::nameToType(fieldName);
	if (type != headerField::Custom) return (find(type));

	const string name = toLower(fieldName);

	// Find the first field that matches the specified name
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && toLower((*pos)->name()) != name ; ++pos);

	// No field with this name can be found
	if (pos == end)
	{
		throw exceptions::no_such_field();
	}
	// Else, return a reference to the existing field
	else
	{
		return (**pos);
	}
}


// Find the first field that matches the specified type/name
headerField& header::fieldsContainer::get(const headerField::Types fieldType)
{
	// Find the first field that matches the specified type
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && (*pos)->type() != fieldType ; ++pos);

	// If no field with this type can be found, create a new one
	if (pos == end)
	{
		headerField* field = headerFieldFactory::getInstance()->create(fieldType);
		insertSorted(field);

		// Return a reference to the new field
		return (*field);
	}
	// Else, return a reference to the existing field
	else
	{
		return (**pos);
	}
}


headerField& header::fieldsContainer::get(const string& fieldName)
{
	headerField::Types type = headerField::nameToType(fieldName);
	if (type != headerField::Custom) return (get(type));

	const string name = toLower(fieldName);

	// Find the first field that matches the specified name
	std::vector <headerField*>::const_iterator pos = m_fields.begin();
	const std::vector <headerField*>::const_iterator end = m_fields.end();

	for ( ; pos != end && toLower((*pos)->name()) != name ; ++pos);

	// If no field with this name can be found, create a new one
	if (pos == end)
	{
		headerField* field = headerFieldFactory::getInstance()->create(fieldName);
		insertSorted(field);

		// Return a reference to the new field
		return (*field);
	}
	// Else, return a reference to the existing field
	else
	{
		return (**pos);
	}
}


void header::fieldsContainer::insertSorted(headerField* field)
{
	const headerField::Types type = field->type();
	std::vector <headerField*>::iterator i;

	for (i = m_fields.begin() ; (i != m_fields.end()) && ((*i)->type() < type) ; ++i);

	m_fields.insert(i, field);
}


// Field insertion
void header::fieldsContainer::append(const headerField& field)
{
	m_fields.push_back(field.clone());
}


void header::fieldsContainer::insert(const iterator it, const headerField& field)
{
	m_fields.insert(it.m_iterator, field.clone());
}


// Field removing
void header::fieldsContainer::remove(const iterator it)
{
	delete (*it.m_iterator);
	m_fields.erase(it.m_iterator);
}


void header::fieldsContainer::clear()
{
	for (std::vector <headerField*>::iterator it = m_fields.begin() ; it != m_fields.end() ; ++it)
		delete (*it);

	m_fields.clear();
}


header::fieldsContainer& header::fieldsContainer::operator=(const fieldsContainer& c)
{
	std::vector <headerField*> fields;

	for (std::vector <headerField*>::const_iterator it = c.m_fields.begin() ; it != c.m_fields.end() ; ++it)
		fields.push_back((*it)->clone());

	for (std::vector <headerField*>::iterator it = m_fields.begin() ; it != m_fields.end() ; ++it)
		delete (*it);

	m_fields.resize(fields.size());
	std::copy(fields.begin(), fields.end(), m_fields.begin());

	return (*this);
}


} // vmime
