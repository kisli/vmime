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

#ifndef VMIME_ENCODERFACTORY_HPP_INCLUDED
#define VMIME_ENCODERFACTORY_HPP_INCLUDED


#include "encoder.hpp"
#include "utility/singleton.hpp"


namespace vmime
{


/** A factory to create 'encoder' objects for the specified encoding.
  */

class encoderFactory : public utility::singleton <encoderFactory>
{
	friend class utility::singleton <encoderFactory>;

private:

	encoderFactory();
	~encoderFactory();

public:

	class registeredEncoder
	{
		friend class encoderFactory;

	protected:

		virtual ~registeredEncoder() { }

	public:

		virtual encoder* create() = 0;

		virtual const string& name() const = 0;
	};

private:

	template <class E>
	class registeredEncoderImpl : public registeredEncoder
	{
		friend class encoderFactory;

	protected:

		registeredEncoderImpl(const string& name) : m_name(name) { }

	public:

		encoder* create()
		{
			return new E;
		}

		const string& name() const
		{
			return (m_name);
		}

	private:

		const string m_name;
	};


	typedef std::map <string, registeredEncoder*> NameMap;
	NameMap m_nameMap;

public:

	template <class E>
	void registerName(const string& name)
	{
		const string _name = toLower(name);
		m_nameMap.insert(NameMap::value_type(_name,
			new registeredEncoderImpl <E>(_name)));
	}

	encoder* create(const string& name);

	const registeredEncoder& operator[](const string& name) const;


	class iterator;

	class const_iterator
	{
		friend class encoderFactory;

	public:

		const_iterator() { }
		const_iterator(const const_iterator& it) : m_it(it.m_it) { }
		const_iterator(const iterator& it) : m_it(it.m_it) { }

		const_iterator& operator=(const const_iterator& it) { m_it = it.m_it; return (*this); }

		const registeredEncoder& operator*() const { return (*(*m_it).second); }
		const registeredEncoder* operator->() const { return ((*m_it).second); }

		const_iterator& operator++() { ++m_it; return (*this); }
		const_iterator operator++(int) { return (m_it++); }

		const_iterator& operator--() { --m_it; return (*this); }
		const_iterator operator--(int) { return (m_it--); }

		const bool operator==(const const_iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const const_iterator& it) const { return (m_it != it.m_it); }

	private:

		const_iterator(const NameMap::const_iterator it) : m_it(it) { }

		NameMap::const_iterator m_it;
	};

	class iterator
	{
		friend class encoderFactory;
		friend class encoderFactory::const_iterator;

	public:

		iterator() { }
		iterator(const iterator& it) : m_it(it.m_it) { }

		iterator& operator=(const iterator& it) { m_it = it.m_it; return (*this); }

		registeredEncoder& operator*() const { return (*(*m_it).second); }
		registeredEncoder* operator->() const { return ((*m_it).second); }

		iterator& operator++() { ++m_it; return (*this); }
		iterator operator++(int) { return (m_it++); }

		iterator& operator--() { --m_it; return (*this); }
		iterator operator--(int) { return (m_it--); }

		const bool operator==(const iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const iterator& it) const { return (m_it != it.m_it); }

	private:

		iterator(const NameMap::iterator it) : m_it(it) { }

		NameMap::iterator m_it;
	};

	iterator begin() { return iterator(m_nameMap.begin()); }
	iterator end() { return iterator(m_nameMap.end()); }

	const_iterator begin() const { return const_iterator(m_nameMap.begin()); }
	const_iterator end() const { return const_iterator(m_nameMap.end()); }
};


} // vmime


#endif // VMIME_ENCODERFACTORY_HPP_INCLUDED
