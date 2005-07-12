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

#ifndef VMIME_TEXTPARTFACTORY_HPP_INCLUDED
#define VMIME_TEXTPARTFACTORY_HPP_INCLUDED


#include "vmime/textPart.hpp"
#include "vmime/mediaType.hpp"


namespace vmime
{


class textPartFactory
{
protected:

	textPartFactory();
	~textPartFactory();

	typedef ref <textPart> (*AllocFunc)(void);
	typedef std::vector <std::pair <mediaType, AllocFunc> > MapType;

	MapType m_map;

#ifndef VMIME_BUILDING_DOC
	template <class TYPE>
	class registerer
	{
	public:

		static ref <textPart> creator()
		{
			// Allocate a new object
			return vmime::create <TYPE>();
		}
	};
#endif // VMIME_BUILDING_DOC

public:

	static textPartFactory* getInstance();

	template <class T>
	void registerType(const mediaType& type)
	{
		m_map.push_back(MapType::value_type(type, &registerer<T>::creator));
	}

	ref <textPart> create(const mediaType& type);
};


} // vmime


#endif // VMIME_TEXTPARTFACTORY_HPP_INCLUDED
