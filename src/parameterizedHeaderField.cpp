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

	getValue().parse(buffer, start, position + (p - pstart));

	removeAllParameters();

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
					appendParameter(parameterFactory::getInstance()->
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

	// Parameters
	for (std::vector <parameter*>::const_iterator
	     it = m_params.begin() ; it != m_params.end() ; ++it)
	{
		os << "; ";
		pos += 2;

		(*it)->generate(os, maxLineLength, pos, &pos);
	}

	if (newLinePos)
		*newLinePos = pos;
}


void parameterizedHeaderField::copyFrom(const component& other)
{
	headerField::copyFrom(other);

	const parameterizedHeaderField& source = dynamic_cast<const parameterizedHeaderField&>(other);

	removeAllParameters();

	for (std::vector <parameter*>::const_iterator i = source.m_params.begin() ;
	     i != source.m_params.end() ; ++i)
	{
		appendParameter((*i)->clone());
	}
}


parameterizedHeaderField& parameterizedHeaderField::operator=(const parameterizedHeaderField& other)
{
	copyFrom(other);
	return (*this);
}


const bool parameterizedHeaderField::hasParameter(const string& paramName) const
{
	const string name = stringUtils::toLower(paramName);

	std::vector <parameter*>::const_iterator pos = m_params.begin();
	const std::vector <parameter*>::const_iterator end = m_params.end();

	for ( ; pos != end && stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	return (pos != end);
}


parameter* parameterizedHeaderField::findParameter(const string& paramName) const
{
	const string name = stringUtils::toLower(paramName);

	// Find the first parameter that matches the specified name
	std::vector <parameter*>::const_iterator pos = m_params.begin();
	const std::vector <parameter*>::const_iterator end = m_params.end();

	for ( ; pos != end && stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	// No parameter with this name can be found
	if (pos == end)
	{
		throw exceptions::no_such_parameter(paramName);
	}
	// Else, return a reference to the existing parameter
	else
	{
		return (*pos);
	}
}


parameter* parameterizedHeaderField::getParameter(const string& paramName)
{
	const string name = stringUtils::toLower(paramName);

	// Find the first parameter that matches the specified name
	std::vector <parameter*>::const_iterator pos = m_params.begin();
	const std::vector <parameter*>::const_iterator end = m_params.end();

	for ( ; pos != end && stringUtils::toLower((*pos)->getName()) != name ; ++pos);

	// If no parameter with this name can be found, create a new one
	if (pos == end)
	{
		parameter* param = parameterFactory::getInstance()->create(paramName);

		try
		{
			appendParameter(param);
		}
		catch (std::exception&)
		{
			delete (param);
			throw;
		}

		// Return a reference to the new parameter
		return (param);
	}
	// Else, return a reference to the existing parameter
	else
	{
		return (*pos);
	}
}


void parameterizedHeaderField::appendParameter(parameter* param)
{
	m_params.push_back(param);
}


void parameterizedHeaderField::insertParameterBefore(parameter* beforeParam, parameter* param)
{
	const std::vector <parameter*>::iterator it = std::find
		(m_params.begin(), m_params.end(), beforeParam);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(beforeParam->getName());

	m_params.insert(it, param);
}


void parameterizedHeaderField::insertParameterBefore(const int pos, parameter* param)
{
	m_params.insert(m_params.begin() + pos, param);
}


void parameterizedHeaderField::insertParameterAfter(parameter* afterParam, parameter* param)
{
	const std::vector <parameter*>::iterator it = std::find
		(m_params.begin(), m_params.end(), afterParam);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(afterParam->getName());

	m_params.insert(it + 1, param);
}


void parameterizedHeaderField::insertParameterAfter(const int pos, parameter* param)
{
	m_params.insert(m_params.begin() + pos + 1, param);
}


void parameterizedHeaderField::removeParameter(parameter* param)
{
	const std::vector <parameter*>::iterator it = std::find
		(m_params.begin(), m_params.end(), param);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(param->getName());

	m_params.erase(it);
}


void parameterizedHeaderField::removeParameter(const int pos)
{
	const std::vector <parameter*>::iterator it = m_params.begin() + pos;

	delete (*it);

	m_params.erase(it);
}


void parameterizedHeaderField::removeAllParameters()
{
	free_container(m_params);
}


const int parameterizedHeaderField::getParameterCount() const
{
	return (m_params.size());
}


const bool parameterizedHeaderField::isEmpty() const
{
	return (m_params.empty());
}


parameter* parameterizedHeaderField::getParameterAt(const int pos)
{
	return (m_params[pos]);
}


const parameter* const parameterizedHeaderField::getParameterAt(const int pos) const
{
	return (m_params[pos]);
}


const std::vector <const parameter*> parameterizedHeaderField::getParameterList() const
{
	std::vector <const parameter*> list;

	list.reserve(m_params.size());

	for (std::vector <parameter*>::const_iterator it = m_params.begin() ;
	     it != m_params.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <parameter*> parameterizedHeaderField::getParameterList()
{
	return (m_params);
}


} // vmime
