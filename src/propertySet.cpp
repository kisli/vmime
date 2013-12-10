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

#include "vmime/propertySet.hpp"
#include "vmime/parserHelpers.hpp"


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
	: object()
{
	for (std::list <shared_ptr <property> >::const_iterator it = set.m_props.begin() ; it != set.m_props.end() ; ++it)
		m_props.push_back(make_shared <property>(**it));
}


propertySet::~propertySet()
{
	removeAllProperties();
}


propertySet& propertySet::operator=(const propertySet& set)
{
	removeAllProperties();

	for (std::list <shared_ptr <property> >::const_iterator it = set.m_props.begin() ; it != set.m_props.end() ; ++it)
		m_props.push_back(make_shared <property>(**it));

	return (*this);
}


void propertySet::setFromString(const string& props)
{
	parse(props);
}


void propertySet::removeAllProperties()
{
	m_props.clear();
}


void propertySet::removeProperty(const string& name)
{
	std::list <shared_ptr <property> >::iterator it = std::find_if
		(m_props.begin(), m_props.end(), propFinder(name));

	if (it != m_props.end())
		m_props.erase(it);
}


void propertySet::parse(const string& props)
{
	const string::const_iterator end = props.end();
	string::const_iterator pos = props.begin();

	for ( ; pos != end ; )
	{
		// Skip white-spaces
		for ( ; pos != end && parserHelpers::isSpace(*pos) ; ++pos) {}

		if (pos != end)
		{
			if (*pos == ';')
			{
				++pos;
				continue;
			}

			// Extract the property name
			const string::const_iterator optStart = pos;

			for ( ; pos != end && *pos != '=' ; ++pos) {}

			string::const_iterator optEnd = pos;

			for ( ; optEnd != optStart && parserHelpers::isSpace(*(optEnd - 1)) ; --optEnd) {}

			const string option(optStart, optEnd);
			string value = "1";

			if (pos != end)
			{
				++pos; // skip '='

				// Extract the value
				for ( ; pos != end && parserHelpers::isSpace(*pos) ; ++pos) {}

				if (pos != end)
				{
					// A quoted-string
					if (*pos == '"' || *pos == '\'')
					{
						value.reserve(50);

						const char quoteChar = *pos;
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

						for ( ; pos != end && !parserHelpers::isSpace(*pos) ; ++pos) {}

						value = string(valStart, pos);
					}

					// Advance to the next ';'
					for ( ; pos != end && (*pos != ';') ; ++pos) {}

					if (pos != end)
						++pos; // skip ';'
				}
			}

			m_props.push_back(make_shared <property>(option, value));
		}
	}
}


shared_ptr <propertySet::property> propertySet::find(const string& name) const
{
	std::list <shared_ptr <property> >::const_iterator it = std::find_if
		(m_props.begin(), m_props.end(), propFinder(name));

	return (it != m_props.end() ? *it : null);
}


shared_ptr <propertySet::property> propertySet::findOrCreate(const string& name)
{
	std::list <shared_ptr <property> >::const_iterator it = std::find_if
		(m_props.begin(), m_props.end(), propFinder(name));

	if (it != m_props.end())
	{
		return (*it);
	}
	else
	{
		shared_ptr <property> prop = make_shared <property>(name, "");
		m_props.push_back(prop);
		return (prop);
	}
}


propertySet::propertyProxy propertySet::operator[](const string& name)
{
	return (propertyProxy(name, this));
}


const propertySet::constPropertyProxy propertySet::operator[](const string& name) const
{
	return (constPropertyProxy(name, this));
}


bool propertySet::hasProperty(const string& name) const
{
	return (find(name) != NULL);
}


const std::vector <shared_ptr <const propertySet::property> > propertySet::getPropertyList() const
{
	std::vector <shared_ptr <const property> > res;

	for (list_type::const_iterator it = m_props.begin() ; it != m_props.end() ; ++it)
		res.push_back(*it);

	return (res);
}


const std::vector <shared_ptr <propertySet::property> > propertySet::getPropertyList()
{
	std::vector <shared_ptr <property> > res;

	for (list_type::const_iterator it = m_props.begin() ; it != m_props.end() ; ++it)
		res.push_back(*it);

	return (res);
}


//
// propertySet::property
//

propertySet::property::property(const string& name, const string& value)
	: m_name(name), m_value(value)
{
}


propertySet::property::property(const string& name)
	: m_name(name)
{
}


propertySet::property::property(const property& prop)
	: object(), m_name(prop.m_name), m_value(prop.m_value)
{
}


const string& propertySet::property::getName() const
{
	return (m_name);
}


#ifndef _MSC_VER


const string& propertySet::property::getValue() const
{
	return (m_value);
}


void propertySet::property::setValue(const string& value)
{
	m_value = value;
}


#endif // !_MSC_VER


#ifndef VMIME_INLINE_TEMPLATE_SPECIALIZATION

template <>
void propertySet::property::setValue(const string& value)
{
	m_value = value;
}


template <>
void propertySet::property::setValue(const bool& value)
{
	m_value = value ? "true" : "false";
}


template <>
string propertySet::property::getValue() const
{
	return (m_value);
}


template <>
bool propertySet::property::getValue() const
{
	if (utility::stringUtils::toLower(m_value) == "true")
		return true;
	else
	{
		int val = 0;

		std::istringstream iss(m_value);
		iss.imbue(std::locale::classic());  // no formatting

		iss >> val;

		return (!iss.fail() && val != 0);
	}
}




template <>
string propertySet::valueFromString(const string& value)
{
	return value;
}


template <>
string propertySet::valueToString(const string& value)
{
	return value;
}


template <>
bool propertySet::valueFromString(const string& value)
{
	if (utility::stringUtils::toLower(value) == "true")
		return true;
	else
	{
		int val = 0;

		std::istringstream iss(value);
		iss.imbue(std::locale::classic());  // no formatting

		iss >> val;

		return (!iss.fail() && val != 0);
	}
}


template <>
string propertySet::valueToString(const bool& value)
{
	return (value ? "true" : "false");
}

#endif // VMIME_INLINE_TEMPLATE_SPECIALIZATION


} // vmime
