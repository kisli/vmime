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

#ifndef VMIME_TEXTPARTFACTORY_HPP_INCLUDED
#define VMIME_TEXTPARTFACTORY_HPP_INCLUDED


#include "vmime/textPart.hpp"
#include "vmime/mediaType.hpp"
#include "vmime/utility/singleton.hpp"


namespace vmime
{


class textPartFactory : public utility::singleton <textPartFactory>
{
	friend class utility::singleton <textPartFactory>;

protected:

	textPartFactory();
	~textPartFactory();

	typedef textPart* (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static textPart* creator()
		{
			// Allocate a new object
			return new TYPE();
		}
	};
#endif // VMIME_BUILDING_DOC

public:

	template <class T>
	void registerType(const mediaType& type)
	{
		m_nameMap.insert(NameMap::value_type(type.generate(), &registerer<T>::creator));
	}

	textPart* create(const mediaType& type);
};


} // vmime


#endif // VMIME_TEXTPARTFACTORY_HPP_INCLUDED
