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

#ifndef VMIME_PARAMETERFACTORY_HPP_INCLUDED
#define VMIME_PARAMETERFACTORY_HPP_INCLUDED


#include "vmime/parameter.hpp"
#include "vmime/utility/singleton.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


class parameterFactory : public utility::singleton <parameterFactory>
{
	friend class utility::singleton <parameterFactory>;

protected:

	parameterFactory();
	~parameterFactory();

	typedef parameter* (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

public:

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static parameter* creator()
		{
			// Allocate a new object
			return new TYPE();
		}
	};
#endif // VMIME_BUILDING_DOC


	template <class T>
	void registerName(const string& name)
	{
		m_nameMap.insert(NameMap::value_type
			(utility::stringUtils::toLower(name), &registerer<T>::creator));
	}

	parameter* create(const string& name, const string& value = NULL_STRING);
};


} // vmime


#endif // VMIME_PARAMETERFACTORY_HPP_INCLUDED
