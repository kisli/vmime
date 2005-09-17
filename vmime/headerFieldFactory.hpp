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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
#define VMIME_HEADERFIELDFACTORY_HPP_INCLUDED


#include "vmime/headerField.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


class headerFieldFactory
{
protected:

	headerFieldFactory();
	~headerFieldFactory();

	typedef ref <headerField> (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

public:

	static headerFieldFactory* getInstance();

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static ref <headerField> creator()
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

	ref <headerField> create(const string& name, const string& body = NULL_STRING);
};


} // vmime


#endif // VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
