//
// VMime library (http://vmime.sourceforge.net)
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

#ifndef VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
#define VMIME_HEADERFIELDFACTORY_HPP_INCLUDED


#include "vmime/headerField.hpp"
#include "vmime/utility/singleton.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


class headerFieldFactory : public utility::singleton <headerFieldFactory>
{
	friend class utility::singleton <headerFieldFactory>;

protected:

	headerFieldFactory();
	~headerFieldFactory();

	typedef headerField* (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

public:

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static headerField* creator()
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

	headerField* create(const string& name, const string& body = NULL_STRING);
};


} // vmime


#endif // VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
