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

#include "parameterizedHeaderField.hpp"
#include "parameterFactory.hpp"
#include "text.hpp"
#include "parserHelpers.hpp"


namespace vmime
{


parameterizedHeaderField::parameterizedHeaderField()
{
}


/*
	This class handles field contents of the following form:
	Field: VALUE; PARAM1="VALUE1"; PARAM2="VALUE2"...

	eg. RFC-1521

     content  :=   "Content-Type"  ":"  type  "/"  subtype  *(";" parameter)

	 parameter := attribute "=" value

     attribute := token   ; case-insensitive

     value := token / quoted-string

     token  :=  1*<any (ASCII) CHAR except SPACE, CTLs, or tspecials>

     tspecials :=  "(" / ")" / "<" / ">" / "@"
                /  "," / ";" / ":" / "\" / <">
                /  "/" / "[" / "]" / "?" / "="
               ; Must be in quoted-string,
               ; to use within parameter values
*/

void parameterizedHeaderField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	const string::size_type start = position;

	while (p < pend && *p != ';') ++p;

	parseValue(buffer, start, position + (p - pstart));

	// If there is one or more parameters following...
	if (p < pend)
	{
		while (*p == ';')
		{
			// Skip ';'
			++p;

			while (p < pend && isspace(*p)) ++p;

			const string::size_type attrStart = position + (p - pstart);

			while (p < pend && !(*p == ';' || *p == '='))
				++p;

			if (p >= pend || *p == ';')
			{
				// Hmmmm... we didn't found an '=' sign.
				// This parameter may not be valid so try to advance
				// to the next one, if there is one.
				while (p < pend && *p != ';')
					++p;
			}
			else
			{
				// Extract the attribute name
				string::size_type attrEnd = position + (p - pstart);

				while (attrEnd != attrStart && isspace(buffer[attrEnd - 1]))
					--attrEnd;

				// Skip '='
				++p;

				// Skip white-spaces between '=' and the value
				while (p < pend && isspace(*p)) ++p;

				// Extract the value
				string value;

				// -- this is a quoted-string
				if (*p == '"')
				{
					// Skip '"'
					++p;

					// Extract quoted-string
					bool escape = false;
					bool stop = false;

					std::ostringstream ss;
					string::size_type start = position + (p - pstart);

					for ( ; p < pend && !stop ; ++p)
					{
						if (escape)
						{
							escape = false;
							start = position + (p - pstart);
						}
						else
						{
							switch (*p)
							{
							case '"':
							{
								ss << string(buffer.begin() + start,
								             buffer.begin() + position + (p - pstart));

								stop = true;
								break;
							}
							case '\\':
							{
								ss << string(buffer.begin() + start,
								             buffer.begin() + position + (p - pstart));

								escape = true;
								break;
							}

							}
						}
					}

					if (!stop)
					{
						ss << string(buffer.begin() + start,
						             buffer.begin() + position + (p - pstart));
					}

					value = ss.str();
				}
				// -- the value is a simple token
				else
				{
					const string::size_type valStart = position + (p - pstart);

					while (p < pend && *p != ';')
						++p;

					string::size_type valEnd = position + (p - pstart);

					while (valEnd != valStart && isspace(buffer[valEnd - 1]))
						--valEnd;

					value = string(buffer.begin() + valStart,
					               buffer.begin() + valEnd);
				}

				// Don't allow ill-formed parameters
				if (attrStart != attrEnd && value.length())
				{
					// Append this parameter to the list
					parameters.m_params.push_back(parameterFactory::getInstance()->
						create(string(buffer.begin() + attrStart,
						              buffer.begin() + attrEnd), value));
				}

				// Skip white-spaces after this parameter
				while (p < pend && isspace(*p)) ++p;
			}
		}
	}

	if (newPosition)
		*newPosition = end;
}


void parameterizedHeaderField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	// Parent header field
	headerField::generate(os, maxLineLength, pos, &pos);

	// Value
	const string value = generateValue();

	encodeAndFoldText(os, text(value), maxLineLength - 1,
		pos, &pos, encodeAndFoldFlags::none);

	// Parameters
	for (std::vector <parameter*>::const_iterator
	     it = parameters.m_params.begin() ; it != parameters.m_params.end() ; ++it)
	{
		const parameter& param = **it;

		os << "; ";
		pos += 2;

		param.generate(os, maxLineLength, pos, &pos);
	}

	if (newLinePos)
		*newLinePos = pos;
}


void parameterizedHeaderField::copyFrom(const headerField& field)
{
	const parameterizedHeaderField& source = dynamic_cast<const parameterizedHeaderField&>(field);

	parameters.clear();

	for (std::vector <parameter*>::const_iterator i = source.parameters.m_params.begin() ;
	     i != source.parameters.m_params.end() ; ++i)
	{
		parameters.m_params.push_back((*i)->clone());
	}

	headerField::copyFrom(field);
}



//////////////////////
// Params container //
//////////////////////


parameterizedHeaderField::paramsContainer::~paramsContainer()
{
	clear();
}


parameter& parameterizedHeaderField::paramsContainer::find(const string& name) const
{
	const string _name = toLower(name);

	std::vector <parameter*>::const_iterator pos = m_params.begin();
	const std::vector <parameter*>::const_iterator end = m_params.end();

	for ( ; pos != end && (*pos)->name() != _name ; ++pos);

	// No parameter with this name can be found
	if (pos == end)
	{
		throw exceptions::no_such_parameter(name);
	}
	// Else, return a reference to the existing parameter
	else
	{
		return (**pos);
	}
}


parameter& parameterizedHeaderField::paramsContainer::get(const string& name)
{
	const string _name = toLower(name);

	std::vector <parameter*>::iterator pos = m_params.begin();
	const std::vector <parameter*>::iterator end = m_params.end();

	for ( ; pos != end && (*pos)->name() != _name ; ++pos);

	// If no parameter with this name can be found, create a new one
	if (pos == end)
	{
		parameter* param = parameterFactory::getInstance()->create(_name);
		m_params.push_back(param);

		// Return a reference to the new parameter
		return (*param);
	}
	// Else, return a reference to the existing parameter
	else
	{
		return (**pos);
	}
}


// Parameter insertion
void parameterizedHeaderField::paramsContainer::append(const parameter& param)
{
	m_params.push_back(param.clone());
}


void parameterizedHeaderField::paramsContainer::insert(const iterator it, const parameter& param)
{
	m_params.insert(it.m_iterator, param.clone());
}


// Parameter removing
void parameterizedHeaderField::paramsContainer::remove(const iterator it)
{
	delete (*it.m_iterator);
	m_params.erase(it.m_iterator);
}


void parameterizedHeaderField::paramsContainer::clear()
{
	free_container(m_params);
}


} // vmime
