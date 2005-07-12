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

#ifndef VMIME_PARAMETERFACTORY_HPP_INCLUDED
#define VMIME_PARAMETERFACTORY_HPP_INCLUDED


#include "vmime/parameter.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


class parameterFactory
{
protected:

	parameterFactory();
	~parameterFactory();

	typedef ref <parameter> (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

public:

	static parameterFactory* getInstance();

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static ref <parameter> creator()
		{
			// Allocate a new object
			return vmime::create <TYPE>();
		}
	};
#endif // VMIME_BUILDING_DOC


	template <class T>
	void registerName(const string& name)
	{
		m_nameMap.insert(NameMap::value_type
			(utility::stringUtils::toLower(name), &registerer<T>::creator));
	}

	/** Create a new parameter and parse its value. The returned parameter
	  * can then be added in a vmime::parameterizedHeaderField object.
	  *
	  * @param name parameter name (ASCII characters only)
	  * @param value value to be parsed
	  * @return created parameter
	  */
	ref <parameter> create(const string& name, const string& value = NULL_STRING);

	/** Create a new parameter and set its value. The returned parameter
	  * can then be added in a vmime::parameterizedHeaderField object.
	  *
	  * @param name parameter name (ASCII characters only)
	  * @param value value to be set
	  * @return created parameter
	  */
	ref <parameter> create(const string& name, const component& value);
};


} // vmime


#endif // VMIME_PARAMETERFACTORY_HPP_INCLUDED
