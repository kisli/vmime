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

#include "base.hpp"
#include "exception.hpp"


namespace vmime
{


class propertySet
{
private:

	class property
	{
	public:

		property(const string& name, const string& value) : m_name(name), m_value(value) { }
		property(const string& name) : m_name(name) { }
		property(const property& prop) : m_name(prop.m_name), m_value(prop.m_value) { }

		const string& name() const { return (m_name); }
		const string& value() const { return (m_value); }

		template <class TYPE> void set(const TYPE& value);
		template <class TYPE> const TYPE get() const;

	private:

		string m_name;
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
			m_set->set(m_name, value);
			return (*this);
		}

		template <class TYPE>
		void set(const TYPE& value)
		{
			m_set->set(m_name, value);
		}

		template <class TYPE>
		const TYPE get() const
		{
			return (m_set->get <TYPE>(m_name));
		}

		operator string() const
		{
			return (m_set->get <string>(m_name));
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
		const TYPE get() const
		{
			return (m_set->get <TYPE>(m_name));
		}

		operator string() const
		{
			return (m_set->get <string>(m_name));
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

	void set(const string& props);

	void empty();

	void clear(const string& name);


	const bool exists(const string& name) const
	{
		return (find(name) != NULL);
	}

	template <class TYPE>
	const TYPE get(const string& name) const
	{
		const property* const prop = find(name);
		if (!prop) throw exceptions::no_such_property(name);

		return (prop->get <TYPE>());
	}

	template <class TYPE>
	const TYPE get(const string& name, const TYPE defaultValue) const
	{
		const property* const prop = find(name);
		return (prop ? prop->get <TYPE>() : defaultValue);
	}

	template <class TYPE>
	void set(const string& name, const TYPE& value)
	{
		findOrCreate(name)->set(value);
	}

	propertyProxy operator[](const string& name)
	{
		return (propertyProxy(name, this));
	}

	const constPropertyProxy operator[](const string& name) const
	{
		return (constPropertyProxy(name, this));
	}

private:

	void parse(const string& props);


	class propFinder : public std::unary_function <property*, bool>
	{
	public:

		propFinder(const string& name) : m_name(toLower(name)) { }

		const bool operator()(property* const p) const
		{
			return (toLower(p->name()) == m_name);
		}

	private:

		const std::string m_name;
	};

	property* find(const string& name) const
	{
		std::list <property*>::const_iterator it = std::find_if
			(m_props.begin(), m_props.end(), propFinder(name));

		return (it != m_props.end() ? *it : NULL);
	}

	property* findOrCreate(const string& name)
	{
		std::list <property*>::const_iterator it = std::find_if
			(m_props.begin(), m_props.end(), propFinder(name));

		if (it != m_props.end())
		{
			return (*it);
		}
		else
		{
			property* prop = new property(name, "");
			m_props.push_back(prop);
			return (prop);
		}
	}

	typedef std::list <property*> list_type;
	list_type m_props;

public:

	class iterator;

	class const_iterator
	{
		friend class propertySet;

	public:

		const_iterator() { }
		const_iterator(const const_iterator& it) : m_it(it.m_it) { }
		const_iterator(const iterator& it) : m_it(it.m_it) { }

		const_iterator& operator=(const const_iterator& it) { m_it = it.m_it; return (*this); }

		const property& operator*() const { return (**m_it); }
		const property* operator->() const { return (*m_it); }

		const_iterator& operator++() { ++m_it; return (*this); }
		const_iterator operator++(int) { return (m_it++); }

		const_iterator& operator--() { --m_it; return (*this); }
		const_iterator operator--(int) { return (m_it--); }

		const bool operator==(const const_iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const const_iterator& it) const { return (m_it != it.m_it); }

	private:

		const_iterator(const list_type::const_iterator it) : m_it(it) { }

		list_type::const_iterator m_it;
	};

	class iterator
	{
		friend class propertySet;
		friend class propertySet::const_iterator;

	public:

		iterator() { }
		iterator(const iterator& it) : m_it(it.m_it) { }

		iterator& operator=(const iterator& it) { m_it = it.m_it; return (*this); }

		property& operator*() const { return (**m_it); }
		property* operator->() const { return (*m_it); }

		iterator& operator++() { ++m_it; return (*this); }
		iterator operator++(int) { return (m_it++); }

		iterator& operator--() { --m_it; return (*this); }
		iterator operator--(int) { return (m_it--); }

		const bool operator==(const iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const iterator& it) const { return (m_it != it.m_it); }

	private:

		iterator(const list_type::iterator it) : m_it(it) { }

		list_type::iterator m_it;
	};

	iterator begin() { return iterator(m_props.begin()); }
	iterator end() { return iterator(m_props.end()); }

	const_iterator begin() const { return const_iterator(m_props.begin()); }
	const_iterator end() const { return const_iterator(m_props.end()); }
};



template <class TYPE>
void propertySet::property::set(const TYPE& value)
{
	std::ostringstream oss;
	oss << value;

	m_value = oss.str();
}


template <class TYPE>
const TYPE propertySet::property::get() const
{
	TYPE val = TYPE();

	std::istringstream iss(m_value);
	iss >> val;

	if (iss.fail())
		throw exceptions::invalid_property_type();

	return (val);
}


template <> void propertySet::property::set(const string& value);
template <> void propertySet::property::set(const bool& value);

template <> const string propertySet::property::get() const;
template <> const bool propertySet::property::get() const;


} // vmime


#endif // VMIME_PROPERTY_HPP_INCLUDED
