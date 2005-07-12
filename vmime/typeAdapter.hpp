//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_TYPEADAPTER_HPP_INCLUDED
#define VMIME_TYPEADAPTER_HPP_INCLUDED


#include <sstream>

#include "vmime/component.hpp"


namespace vmime
{


/** An adapter to allow any type to act as a 'component'.
  */

template <class TYPE>
class typeAdapter : public component
{
public:

	typeAdapter()
	{
	}

	typeAdapter(typeAdapter& a)
		: component(), m_value(a.m_value)
	{
	}

	typeAdapter(const TYPE& v)
		: m_value(v)
	{
	}


	ref <component> clone() const
	{
		return create <typeAdapter>(*this);
	}


	void copyFrom(const component& other)
	{
		m_value = dynamic_cast <const typeAdapter <TYPE>&>(other).m_value;
	}


	typeAdapter& operator=(const TYPE& v)
	{
		m_value = v;
		return (*this);
	}


	typeAdapter& operator=(const component& other)
	{
		copyFrom(other);
		return (*this);
	}


	operator TYPE() const
	{
		return (m_value);
	}


	void parse(const string& buffer, const string::size_type position,
	           const string::size_type end, string::size_type* newPosition = NULL)
	{
		std::istringstream iss(string(buffer.begin() + position, buffer.begin() + end));
		iss >> m_value;

		setParsedBounds(position, end);

		if (newPosition)
			*newPosition = end;
	}


	void generate(utility::outputStream& os,
	              const string::size_type /* maxLineLength */ = lineLengthLimits::infinite,
	              const string::size_type curLinePos = 0,
	              string::size_type* newLinePos = NULL) const
	{
		std::ostringstream oss;
		oss << m_value;

		os << oss.str();

		if (newLinePos)
			*newLinePos = curLinePos + oss.str().length();
	}

	const std::vector <ref <const component> > getChildComponents() const
	{
		return std::vector <ref <const component> >();
	}

private:

	TYPE m_value;
};


#if (defined(__GNUC__) && (__GNUC__ >= 3) && (__GNUC_MINOR__ <= 2)) || defined(_MSC_VER)

	// Because of a bug with g++ <= 3.2, we have to put the implementation
	// of the function inline.

	template <>
	inline void typeAdapter <string>::parse
		(const string& buffer, const string::size_type position,
		 const string::size_type end, string::size_type* newPosition)
	{
		m_value = string(buffer.begin() + position, buffer.begin() + end);

		if (newPosition)
			*newPosition = end;
	}

#else

	template <>
	void typeAdapter <string>::parse
		(const string& buffer, const string::size_type position,
		 const string::size_type end, string::size_type* newPosition);

#endif // (defined(__GNUC__) && (__GNUC__ >= 3) && (__GNUC_MINOR__ <= 2)) || defined(_MSC_VER)


} // vmime


#endif // VMIME_TYPEADAPTE_HPP_INCLUDED
