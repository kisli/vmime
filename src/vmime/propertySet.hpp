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

class VMIME_EXPORT propertySet : public object
{
public:

	/** A property holds a (name,value) pair.
	  */
	class property : public object
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

#ifndef _MSC_VER

		// Visual Studio errors on linking with these 2 functions,
		// whereas GCC and CLang need them.

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

#endif  // !_MSC_VER

		/** Set the value of the property as a generic type.
		  *
		  * @param value new value for property
		  */
		template <class TYPE> void setValue(const TYPE& value)
		{
			std::ostringstream oss;
			oss.imbue(std::locale::classic());  // no formatting

			oss << value;

			m_value = oss.str();
		}

		/** Get the value of the property as a generic type.
		  *
		  * @throw exceptions::invalid_property_type if the specified
		  * type is incompatible with the string value (cannot be
		  * converted using std::istringstream)
		  * @return current value of the property
		  */
		template <class TYPE> TYPE getValue() const
		{
			TYPE val = TYPE();

			std::istringstream iss(m_value);
			iss.imbue(std::locale::classic());  // no formatting

			iss >> val;

			if (iss.fail())
				throw exceptions::invalid_property_type();

			return (val);
		}


#ifdef VMIME_INLINE_TEMPLATE_SPECIALIZATION

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

#endif // VMIME_INLINE_TEMPLATE_SPECIALIZATION

	private:

		const string m_name;
		string m_value;
	};

protected:

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
	bool hasProperty(const string& name) const;

	/** Get the value of the specified property.
	  *
	  * @throw exceptions::no_such_property if the property does not exist
	  * @param name property name
	  * @return value of the specified property
	  */
	template <class TYPE>
	const TYPE getProperty(const string& name) const
	{
		const shared_ptr <property> prop = find(name);
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
		const shared_ptr <property> prop = find(name);
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


	class propFinder : public std::unary_function <shared_ptr <property>, bool>
	{
	public:

		propFinder(const string& name) : m_name(utility::stringUtils::toLower(name)) { }

		bool operator()(shared_ptr <property> p) const
		{
			return (utility::stringUtils::toLower(p->getName()) == m_name);
		}

	private:

		const string m_name;
	};

	shared_ptr <property> find(const string& name) const;
	shared_ptr <property> findOrCreate(const string& name);

	typedef std::list <shared_ptr <property> > list_type;
	list_type m_props;

public:

	template <typename TYPE>
	static TYPE valueFromString(const string& value)
	{
		TYPE v = TYPE();

		std::istringstream iss(value);
		iss.imbue(std::locale::classic());  // no formatting

		iss >> v;

		return v;
	}

	template <typename TYPE>
	static string valueToString(const TYPE& value)
	{
		std::ostringstream oss(value);
		oss.imbue(std::locale::classic());  // no formatting

		oss << value;

		return oss.str();
	}

#ifdef VMIME_INLINE_TEMPLATE_SPECIALIZATION

	template <>
	static string valueFromString(const string& value)
	{
		return value;
	}

	template <>
	static string valueToString(const string& value)
	{
		return value;
	}

	template <>
	static bool valueFromString(const string& value)
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
	static string valueToString(const bool& value)
	{
		return (value ? "true" : "false");
	}

#endif // VMIME_INLINE_TEMPLATE_SPECIALIZATION

	/** Return the property list.
	  *
	  * @return list of properties
	  */
	const std::vector <shared_ptr <const property> > getPropertyList() const;

	/** Return the property list.
	  *
	  * @return list of properties
	  */
	const std::vector <shared_ptr <property> > getPropertyList();
};


#ifndef VMIME_INLINE_TEMPLATE_SPECIALIZATION

template <> VMIME_EXPORT void propertySet::property::setValue <string>(const string& value);
template <> VMIME_EXPORT void propertySet::property::setValue(const bool& value);

template <> VMIME_EXPORT string propertySet::property::getValue() const;
template <> VMIME_EXPORT bool propertySet::property::getValue() const;

template <> VMIME_EXPORT string propertySet::valueFromString(const string& value);
template <> VMIME_EXPORT string propertySet::valueToString(const string& value);

template <> VMIME_EXPORT bool propertySet::valueFromString(const string& value);
template <> VMIME_EXPORT string propertySet::valueToString(const bool& value);

#endif // VMIME_INLINE_TEMPLATE_SPECIALIZATION


} // vmime


#endif // VMIME_PROPERTY_HPP_INCLUDED
