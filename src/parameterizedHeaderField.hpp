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

#ifndef VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED
#define VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED


#include "base.hpp"
#include "headerFieldFactory.hpp"
#include "parameter.hpp"
#include "exception.hpp"
#include "parameterFactory.hpp"


namespace vmime
{


class parameterizedHeaderField : public headerField
{
	friend class headerFieldFactory::registerer <parameterizedHeaderField>;

protected:

	parameterizedHeaderField();

public:

	void copyFrom(const headerField& field);

	// A sub-class for parameter manipulation
	class paramsContainer
	{
		friend class parameterizedHeaderField;

	protected:

		~paramsContainer();

	public:

		// Find the first parameter with the specified name. If no parameter
		// is found, an exception is thrown.
		parameter& find(const string& name) const;

		// Find the first parameter with the specified name
		parameter& get(const string& name);

		// Parameter iterator
		class const_iterator;

		class iterator
		{
			friend class parameterizedHeaderField::paramsContainer::const_iterator;
			friend class parameterizedHeaderField::paramsContainer;

		public:

			typedef std::vector <parameter*>::iterator::difference_type difference_type;

			iterator(std::vector <parameter*>::iterator it) : m_iterator(it) { }
			iterator(const iterator& it) : m_iterator(it.m_iterator) { }

			iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			parameter& operator*() const { return (**m_iterator); }
			parameter* operator->() const { return (*m_iterator); }

			iterator& operator++() { ++m_iterator; return (*this); }
			iterator operator++(int) { iterator i(*this); ++m_iterator; return (i); }

			iterator& operator--() { --m_iterator; return (*this); }
			iterator operator--(int) { iterator i(*this); --m_iterator; return (i); }

			iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			iterator operator-(difference_type x) const { return iterator(m_iterator - x); }

			parameter& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <parameter*>::iterator m_iterator;
		};

		class const_iterator
		{
		public:

			typedef std::vector <parameter*>::const_iterator::difference_type difference_type;

			const_iterator(std::vector <parameter*>::const_iterator it) : m_iterator(it) { }
			const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
			const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

			const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
			const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			const parameter& operator*() const { return (**m_iterator); }
			const parameter* operator->() const { return (*m_iterator); }

			const_iterator& operator++() { ++m_iterator; return (*this); }
			const_iterator operator++(int) { const_iterator i(*this); ++m_iterator; return (i); }

			const_iterator& operator--() { --m_iterator; return (*this); }
			const_iterator operator--(int) { const_iterator i(*this); --m_iterator; return (i); }

			const_iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			const_iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			const_iterator operator-(difference_type x) const { return const_iterator(m_iterator - x); }

			const parameter& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <parameter*>::const_iterator m_iterator;
		};

	public:

		iterator begin() { return (m_params.begin()); }
		iterator end() { return (m_params.end()); }

		const_iterator begin() const { return (const_iterator(m_params.begin())); }
		const_iterator end() const { return (const_iterator(m_params.end())); }

		// Parameter insertion
		void append(const parameter& param);
		void insert(const iterator it, const parameter& param);

		// Parameter removing
		void remove(const iterator it);
		void clear();

		// Parameter count
		const size_t count() const { return (m_params.size()); }
		const size_t size() const { return (m_params.size()); }

		parameter& front() { return (*m_params.front()); }
		const parameter& front() const { return (*m_params.front()); }
		parameter& back() { return (*m_params.back()); }
		const parameter& back() const { return (*m_params.back()); }

	protected:

		std::vector <parameter*> m_params;

	} parameters;

	typedef paramsContainer::iterator iterator;
	typedef paramsContainer::const_iterator const_iterator;

protected:

	std::vector <parameter*> m_params;

protected:

	virtual void parseValue(const string& buffer, const string::size_type position, const string::size_type end) = 0;
	virtual const string generateValue() const = 0;

public:

	using headerField::parse;
	using headerField::generate;

	// No need to override these (use "parseValue" and "generateValue" instead).
	// For more information, see "defaultParameter.hpp".
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED
