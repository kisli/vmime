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

#ifndef VMIME_PROPERTY_HPP_INCLUDED
#define VMIME_PROPERTY_HPP_INCLUDED


#include <list>
#include <functional>
#include <algorithm>
#include <sstream>

#include "vmime/base.hpp"
#include "vmime/exception.hpp"

#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


/** Manage a list of (name,value) pairs.
  */

class propertySet
{
protected:

	class property
	{
	public:

		property(const string& name, const string& value);
		property(const string& name);
		property(const property& prop);

		/** Return the name of the property.
		  *
		  * @return property name
		  */
		const string& getName() const;

		/** Return the value of the property as a string.
		  *
		  * @return current value of the property
		  */
		const string& getValue() const;

		/** Set the value of the property as a string.
		  *
		  * @param value new value for property
		  */
		void setValue(const string& value);

		/** Set the value of the property as a generic type.
		  *
		  * @param value new value for property
		  */
		template <class TYPE> void setValue(const TYPE& value);

		/** Get the value of the property as a generic type.
		  *
		  * @throw exceptions::invalid_property_type if the specified
		  * type is incompatible with the string value (cannot be
		  * converted using std::istringstream)
		  * @return current value of the property
		  */
		template <class TYPE> const TYPE getValue() const;

	private:

		const string m_name;
		string m_value;
	};

	class propertyProxy
	{
	public:

		propertyProxy(const string& name, propertySet* set)
			: m_name(name), m_set(set)
		{
		}

		template <class TYPE>
		propertyProxy& operator=(const TYPE& value)
		{
			m_set->setProperty(m_name, value);
			return (*this);
		}

		template <class TYPE>
		void setValue(const TYPE& value)
		{
			m_set->setProperty(m_name, value);
		}

		template <class TYPE>
		const TYPE getValue() const
		{
			return (m_set->getProperty <TYPE>(m_name));
		}

		operator string() const
		{
			return (m_set->getProperty <string>(m_name));
		}

	private:

		const string m_name;
		propertySet* m_set;
	};

	class constPropertyProxy
	{
	public:

		constPropertyProxy(const string& name, const propertySet* set)
			: m_name(name), m_set(set)
		{
		}

		template <class TYPE>
		const TYPE getValue() const
		{
			return (m_set->getProperty <TYPE>(m_name));
		}

		operator string() const
		{
			return (m_set->getProperty <string>(m_name));
		}

	private:

		const string m_name;
		const propertySet* m_set;
	};

public:

	propertySet();
	propertySet(const string& props);
	propertySet(const propertySet& set);

	~propertySet();

	propertySet& operator=(const propertySet& set);

	/** Parse a string and extract one or more properties.
	  * The string format is: name[=value](;name[=value])*.
	  *
	  * @param props string representing a list of properties
	  */
	void setFromString(const string& props);

	/** Remove all properties from the list.
	  */
	void removeAllProperties();

	/** Remove the specified property.
	  *
	  * @param name name of the property to remove
	  */
	void removeProperty(const string& name);

	/** Test whether the specified property is set.
	  *
	  * @param name name of the property to test
	  * @return true if the property is set (has a value),
	  * false otherwise
	  */
	const bool hasProperty(const string& name) const;

	/** Get the value of the specified property.
	  *
	  * @throw exceptions::no_such_property if the property does not exist
	  * @param name property name
	  * @return value of the specified property
	  */
	template <class TYPE>
	const TYPE getProperty(const string& name) const
	{
		const property* const prop = find(name);
		if (!prop) throw exceptions::no_such_property(name);

		//return (prop->getValue <TYPE>());  // BUG: with g++ < 3.4
		return (prop->template getValue <TYPE>());
	}

	/** Get the value of the specified property.
	  * A default value can be returned if the property is not set.
	  *
	  * @param name property name
	  * @param defaultValue value to return if the specified property
	  * does not exist
	  * @return value of the specified property or default value
	  * if if does not exist
	  */
	template <class TYPE>
	const TYPE getProperty(const string& name, const TYPE defaultValue) const
	{
		const property* const prop = find(name);
		//return (prop ? prop->getValue <TYPE>() : defaultValue); // BUG: with g++ < 3.4
		return (prop ? prop->template getValue <TYPE>() : defaultValue);
	}

	/** Change the value of the specified property or create
	  * a new property set to the specified a value.
	  *
	  * @param name property name
	  * @param value property value
	  */
	template <class TYPE>
	void setProperty(const string& name, const TYPE& value)
	{
		findOrCreate(name)->setValue(value);
	}

	/** Return a proxy object to access the specified property
	  * suitable for reading or writing. If the property does not
	  * exist and the value is changed, a new property will
	  * be created.
	  *
	  * @param name property name
	  * @return proxy object for the specified property
	  */
	propertyProxy operator[](const string& name);

	/** Return a proxy object to access the specified property
	  * suitable for reading only.
	  *
	  * @throw exceptions::no_such_property if the property does not exist
	  * @return read-only proxy object for the specified property
	  */
	const constPropertyProxy operator[](const string& name) const;

private:

	void parse(const string& props);


	class propFinder : public std::unary_function <property*, bool>
	{
	public:

		propFinder(const string& name) : m_name(utility::stringUtils::toLower(name)) { }

		const bool operator()(property* const p) const
		{
			return (utility::stringUtils::toLower(p->getName()) == m_name);
		}

	private:

		const std::string m_name;
	};

	property* find(const string& name) const;
	property* findOrCreate(const string& name);

	typedef std::list <property*> list_type;
	list_type m_props;

public:

	/** Return the property list.
	  *
	  * @return list of properties
	  */
	const std::vector <const property*> getPropertyList() const;

	/** Return the property list.
	  *
	  * @return list of properties
	  */
	const std::vector <property*> getPropertyList();
};



template <class TYPE>
void propertySet::property::setValue(const TYPE& value)
{
	std::ostringstream oss;
	oss << value;

	m_value = oss.str();
}


template <class TYPE>
const TYPE propertySet::property::getValue() const
{
	TYPE val = TYPE();

	std::istringstream iss(m_value);
	iss >> val;

	if (iss.fail())
		throw exceptions::invalid_property_type();

	return (val);
}


template <> void propertySet::property::setValue(const string& value);
template <> void propertySet::property::setValue(const bool& value);

template <> const string propertySet::property::getValue() const;
template <> const bool propertySet::property::getValue() const;


} // vmime


#endif // VMIME_PROPERTY_HPP_INCLUDED
