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

#include "propertySet.hpp"


namespace vmime
{


propertySet::propertySet()
{
}


propertySet::propertySet(const string& props)
{
	parse(props);
}


propertySet::propertySet(const propertySet& set)
{
	for (std::list <property*>::const_iterator it = set.m_props.begin() ; it != set.m_props.end() ; ++it)
		m_props.push_back(new property(**it));
}


propertySet::~propertySet()
{
	empty();
}


propertySet& propertySet::operator=(const propertySet& set)
{
	empty();

	for (std::list <property*>::const_iterator it = set.m_props.begin() ; it != set.m_props.end() ; ++it)
		m_props.push_back(new property(**it));

	return (*this);
}


void propertySet::set(const string& props)
{
	parse(props);
}


void propertySet::empty()
{
	free_container(m_props);
}


void propertySet::clear(const string& name)
{
	std::list <property*>::iterator it = std::find_if
		(m_props.begin(), m_props.end(), propFinder(name));

	if (it != m_props.end())
	{
		delete (*it);
		m_props.erase(it);
	}
}


void propertySet::parse(const string& props)
{
	const string::const_iterator end = props.end();
	string::const_iterator pos = props.begin();

	for ( ; pos != end ; )
	{
		// Skip white-spaces
		for ( ; pos != end && isspace(*pos) ; ++pos);

		if (pos != end)
		{
			if (*pos == ';')
			{
				++pos;
				continue;
			}

			// Extract the property name
			const string::const_iterator optStart = pos;

			for ( ; pos != end && *pos != '=' ; ++pos);

			string::const_iterator optEnd = pos;

			for ( ; optEnd != optStart && isspace(*(optEnd - 1)) ; --optEnd);

			const string option(optStart, optEnd);
			string value = "1";

			if (pos != end)
			{
				++pos; // skip '='

				// Extract the value
				for ( ; pos != end && isspace(*pos) ; ++pos);

				if (pos != end)
				{
					// A quoted-string
					if (*pos == '"' || *pos == '\'')
					{
						value.reserve(50);

						const std::string::value_type quoteChar = *pos;
						bool theEnd = false;
						bool escape = false;

						for ( ; (pos != end) && !theEnd ; ++pos)
						{
							if (escape)
							{
								value += *pos;
								escape = false;
							}
							else
							{
								if (*pos == '\\')
									escape = true;
								else if (*pos == quoteChar)
									theEnd = true;
								else
									value += *pos;
							}
						}

						if (pos != end)
							++pos;
					}
					// Simple value
					else
					{
						const string::const_iterator valStart = pos;

						for ( ; pos != end && !isspace(*pos) ; ++pos);

						value = string(valStart, pos);
					}

					// Advance to the next ';'
					for ( ; pos != end && (*pos != ';') ; ++pos);

					if (pos != end)
						++pos; // skip ';'
				}
			}

			m_props.push_back(new property(option, value));
		}
	}
}


template <>
void propertySet::property::set(const string& value)
{
	m_value = value;
}


template <>
void propertySet::property::set(const bool& value)
{
	m_value = value ? "true" : "false";
}


template <>
const string propertySet::property::get() const
{
	return (m_value);
}


template <>
const bool propertySet::property::get() const
{
	if (toLower(m_value) == "true")
		return true;
	else
	{
		int val = 0;

		std::istringstream iss(m_value);
		iss >> val;

		return (!iss.fail() && val != 0);
	}
}


} // vmime
