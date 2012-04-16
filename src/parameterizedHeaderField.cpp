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

#include "vmime/parameterizedHeaderField.hpp"
#include "vmime/text.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


parameterizedHeaderField::parameterizedHeaderField()
{
}


parameterizedHeaderField::~parameterizedHeaderField()
{
	removeAllParameters();
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


#ifndef VMIME_BUILDING_DOC

struct paramInfo
{
	bool extended;
	std::vector <parameter::valueChunk> value;
	string::size_type start;
	string::size_type end;
};

#endif // VMIME_BUILDING_DOC


void parameterizedHeaderField::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	// Skip non-significant whitespaces
	string::size_type valueStart = position;

	while (p < pend && parserHelpers::isSpace(*p))
	{
		++p;
		++valueStart;
	}

	// Advance up to ';', if any
	string::size_type valueLength = 0;

	while (p < pend && *p != ';')  // FIXME: support ";" inside quoted or RFC-2047-encoded text
	{
		++p;
		++valueLength;
	}

	// Trim whitespaces at the end of the value
	while (valueLength > 0 && parserHelpers::isSpace(buffer[valueStart + valueLength - 1]))
		--valueLength;

	// Parse value
	getValue()->parse(buffer, valueStart, valueStart + valueLength);

	// Reset parameters
	removeAllParameters();

	// If there is one or more parameters following...
	if (p < pend)
	{
		std::map <string, paramInfo> params;

		while (*p == ';')
		{
			// Skip ';'
			++p;

			while (p < pend && parserHelpers::isSpace(*p)) ++p;

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

				while (attrEnd != attrStart && parserHelpers::isSpace(buffer[attrEnd - 1]))
					--attrEnd;

				// Skip '='
				++p;

				// Skip white-spaces between '=' and the value
				while (p < pend && parserHelpers::isSpace(*p)) ++p;

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

					while (valEnd != valStart && parserHelpers::isSpace(buffer[valEnd - 1]))
						--valEnd;

					value = string(buffer.begin() + valStart,
					               buffer.begin() + valEnd);
				}

				// Don't allow ill-formed parameters
				if (attrStart != attrEnd && value.length())
				{
					string name(buffer.begin() + attrStart, buffer.begin() + attrEnd);

					// Check for RFC-2231 extended parameters
					bool extended = false;
					bool encoded = false;

					if (name[name.length() - 1] == '*')
					{
						name.erase(name.end() - 1, name.end());

						extended = true;
						encoded = true;
					}

					// Check for RFC-2231 multi-section parameters
					const string::size_type star = name.find_last_of('*');

					if (star != string::npos)
					{
						bool allDigits = true;

						for (string::size_type i = star + 1 ; allDigits && (i < name.length()) ; ++i)
							allDigits = parserHelpers::isDigit(name[i]);

						if (allDigits)
						{
							name.erase(name.begin() + star, name.end());
							extended = true;
						}

						// NOTE: we ignore section number, and we suppose that
						// the sequence is correct (ie. the sections appear
						// in order: param*0, param*1...)
					}

					// Add/replace/modify the parameter
					const std::map <string, paramInfo>::iterator it = params.find(name);

					if (it != params.end())
					{
						paramInfo& info = (*it).second;

						// An extended parameter replaces a normal one
						if (!info.extended)
						{
							info.extended = extended;
							info.value.clear();
							info.start = attrStart;
						}

						// Append a new section for a multi-section parameter
						parameter::valueChunk chunk;
						chunk.encoded = encoded;
						chunk.data = value;

						info.value.push_back(chunk);
						info.end = position + (p - pstart);
					}
					else
					{
						parameter::valueChunk chunk;
						chunk.encoded = encoded;
						chunk.data = value;

						paramInfo info;
						info.extended = extended;
						info.value.push_back(chunk);
						info.start = attrStart;
						info.end = position + (p - pstart);

						// Insert a new parameter
						params.insert(std::map <string, paramInfo>::value_type(name, info));
					}
				}

				// Skip white-spaces after this parameter
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
			}
		}

		for (std::map <string, paramInfo>::const_iterator it = params.begin() ;
		     it != params.end() ; ++it)
		{
			const paramInfo& info = (*it).second;

			// Append this parameter to the list
			ref <parameter> param = vmime::create <parameter>((*it).first);

			param->parse(info.value);
			param->setParsedBounds(info.start, info.end);

			appendParameter(param);
		}
	}

	if (newPosition)
		*newPosition = end;
}


void parameterizedHeaderField::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	// Parent header field
	headerField::generateImpl(os, maxLineLength, pos, &pos);

	// Parameters
	for (std::vector <ref <parameter> >::const_iterator
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

	for (std::vector <ref <parameter> >::const_iterator i = source.m_params.begin() ;
	     i != source.m_params.end() ; ++i)
	{
		appendParameter((*i)->clone().dynamicCast <parameter>());
	}
}


parameterizedHeaderField& parameterizedHeaderField::operator=(const parameterizedHeaderField& other)
{
	copyFrom(other);
	return (*this);
}


bool parameterizedHeaderField::hasParameter(const string& paramName) const
{
	const string name = utility::stringUtils::toLower(paramName);

	std::vector <ref <parameter> >::const_iterator pos = m_params.begin();
	const std::vector <ref <parameter> >::const_iterator end = m_params.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos) {}

	return (pos != end);
}


ref <parameter> parameterizedHeaderField::findParameter(const string& paramName) const
{
	const string name = utility::stringUtils::toLower(paramName);

	// Find the first parameter that matches the specified name
	std::vector <ref <parameter> >::const_iterator pos = m_params.begin();
	const std::vector <ref <parameter> >::const_iterator end = m_params.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos) {}

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


ref <parameter> parameterizedHeaderField::getParameter(const string& paramName)
{
	const string name = utility::stringUtils::toLower(paramName);

	// Find the first parameter that matches the specified name
	std::vector <ref <parameter> >::const_iterator pos = m_params.begin();
	const std::vector <ref <parameter> >::const_iterator end = m_params.end();

	for ( ; pos != end && utility::stringUtils::toLower((*pos)->getName()) != name ; ++pos) {}

	// If no parameter with this name can be found, create a new one
	if (pos == end)
	{
		ref <parameter> param = vmime::create <parameter>(paramName);

		appendParameter(param);

		// Return a reference to the new parameter
		return (param);
	}
	// Else, return a reference to the existing parameter
	else
	{
		return (*pos);
	}
}


void parameterizedHeaderField::appendParameter(ref <parameter> param)
{
	m_params.push_back(param);
}


void parameterizedHeaderField::insertParameterBefore(ref <parameter> beforeParam, ref <parameter> param)
{
	const std::vector <ref <parameter> >::iterator it = std::find
		(m_params.begin(), m_params.end(), beforeParam);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(beforeParam->getName());

	m_params.insert(it, param);
}


void parameterizedHeaderField::insertParameterBefore(const int pos, ref <parameter> param)
{
	m_params.insert(m_params.begin() + pos, param);
}


void parameterizedHeaderField::insertParameterAfter(ref <parameter> afterParam, ref <parameter> param)
{
	const std::vector <ref <parameter> >::iterator it = std::find
		(m_params.begin(), m_params.end(), afterParam);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(afterParam->getName());

	m_params.insert(it + 1, param);
}


void parameterizedHeaderField::insertParameterAfter(const int pos, ref <parameter> param)
{
	m_params.insert(m_params.begin() + pos + 1, param);
}


void parameterizedHeaderField::removeParameter(ref <parameter> param)
{
	const std::vector <ref <parameter> >::iterator it = std::find
		(m_params.begin(), m_params.end(), param);

	if (it == m_params.end())
		throw exceptions::no_such_parameter(param->getName());

	m_params.erase(it);
}


void parameterizedHeaderField::removeParameter(const int pos)
{
	const std::vector <ref <parameter> >::iterator it = m_params.begin() + pos;

	m_params.erase(it);
}


void parameterizedHeaderField::removeAllParameters()
{
	m_params.clear();
}


int parameterizedHeaderField::getParameterCount() const
{
	return (m_params.size());
}


bool parameterizedHeaderField::isEmpty() const
{
	return (m_params.empty());
}


const ref <parameter> parameterizedHeaderField::getParameterAt(const int pos)
{
	return (m_params[pos]);
}


const ref <const parameter> parameterizedHeaderField::getParameterAt(const int pos) const
{
	return (m_params[pos]);
}


const std::vector <ref <const parameter> > parameterizedHeaderField::getParameterList() const
{
	std::vector <ref <const parameter> > list;

	list.reserve(m_params.size());

	for (std::vector <ref <parameter> >::const_iterator it = m_params.begin() ;
	     it != m_params.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <ref <parameter> > parameterizedHeaderField::getParameterList()
{
	return (m_params);
}


const std::vector <ref <component> > parameterizedHeaderField::getChildComponents()
{
	std::vector <ref <component> > list = headerField::getChildComponents();

	for (std::vector <ref <parameter> >::iterator it = m_params.begin() ;
	     it != m_params.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


} // vmime

