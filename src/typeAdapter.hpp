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

#ifndef VMIME_TYPEADAPTER_HPP_INCLUDED
#define VMIME_TYPEADAPTER_HPP_INCLUDED


#include <sstream>

#include "component.hpp"


namespace vmime
{


template <class TYPE>
class typeAdapter : public component
{
public:

	typeAdapter()
	{
	}

	typeAdapter(typeAdapter& a)
		: m_value(a.m_value)
	{
	}

	typeAdapter(const TYPE& v)
		: m_value(v)
	{
	}


	typeAdapter* clone() const
	{
		return new typeAdapter(*this);
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

private:

	TYPE m_value;
};


} // vmime


#endif // VMIME_TYPEADAPTE_HPP_INCLUDED
