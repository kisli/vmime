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

#ifndef VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
#define VMIME_HEADERFIELDFACTORY_HPP_INCLUDED


#include "headerField.hpp"
#include "utility/singleton.hpp"


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
	typedef std::map <headerField::Types, AllocFunc> TypeMap;

	NameMap m_nameMap;
	TypeMap m_typeMap;

public:

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


	template <class T>
	void registerName(const string& name)
	{
		m_nameMap.insert(NameMap::value_type(toLower(name), &registerer<T>::creator));
	}

	headerField* create(const string& name, const string& body = NULL_STRING);
	headerField* create(const headerField::Types type, const string& name = NULL_STRING, const string& body = NULL_STRING);

protected:

	template <class T>
	void registerType(const headerField::Types type)
	{
		m_typeMap.insert(TypeMap::value_type(type, &registerer<T>::creator));
	}
};


} // vmime


#endif // VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
