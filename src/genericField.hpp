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

#ifndef VMIME_GENERICFIELD_HPP_INCLUDED
#define VMIME_GENERICFIELD_HPP_INCLUDED


#include "headerField.hpp"
#include "headerFieldFactory.hpp"

#include "typeAdapter.hpp"


namespace vmime
{


/** Generic implementation for headerField.
  */

template <class VALUE_TYPE>
class genericField : virtual public headerField
{
	friend class headerFieldFactory::registerer <genericField <VALUE_TYPE> >;

protected:

	genericField() { }

public:

	genericField <VALUE_TYPE>& operator=(const genericField <VALUE_TYPE>& other)
	{
		copyFrom(other);
		return (*this);
	}

	const VALUE_TYPE& getValue() const
	{
		return (m_value);
	}

	VALUE_TYPE& getValue()
	{
		return (m_value);
	}

	template <class TYPE>
	void setValue(const TYPE& value)
	{
		m_value = value;
	}

	void setValue(const component& value)
	{
		const VALUE_TYPE& v = dynamic_cast <const VALUE_TYPE&>(value);
		m_value = v;
	}

private:

	VALUE_TYPE m_value;
};


template <>
class genericField <string> : public genericField <typeAdapter <string> >
{
};


} // vmime


#endif // VMIME_GENERICFIELD_HPP_INCLUDED
