//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
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


/** Creates header field and header field value objects.
  */

class headerFieldFactory
{
protected:

	headerFieldFactory();
	~headerFieldFactory();

	typedef ref <headerField> (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;

	typedef ref <headerFieldValue> (*ValueAllocFunc)(void);
	typedef std::map <string, ValueAllocFunc> ValueMap;

	ValueMap m_valueMap;

public:

	static headerFieldFactory* getInstance();

#ifndef VMIME_BUILDING_DOC
	// TYPE must inherit from BASE_TYPE
	template <class BASE_TYPE, class TYPE>
	class registerer
	{
	public:

		static ref <BASE_TYPE> creator()
		{
			// Allocate a new object
			return vmime::create <TYPE>();
		}
	};
#endif // VMIME_BUILDING_DOC


	/** Register a field type.
	  *
	  * @param T field class (must inherit from 'headerField')
	  * @param name field name (eg. "X-MyField")
	  */
	template <class T>
	void registerField(const string& name)
	{
		m_nameMap.insert(NameMap::value_type
			(utility::stringUtils::toLower(name),
			 &registerer <headerField, T>::creator));
	}

	/** Register a field value type.
	  *
	  * @param T value class (must inherit from 'headerFieldValue')
	  * @param name field name
	  */
	template <class T>
	void registerFieldValue(const string& name)
	{
		m_valueMap.insert(ValueMap::value_type
			(utility::stringUtils::toLower(name),
			 &registerer <headerFieldValue, T>::creator));
	}

	/** Create a new field object for the specified field name.
	  * If the field name has not been registered, a default type
	  * is used.
	  *
	  * @param name field name
	  * @param body string that will be parsed to initialize
	  * the value of the field
	  * @return a new field object
	  */
	ref <headerField> create(const string& name, const string& body = NULL_STRING);

	/** Create a new field value for the specified field.
	  *
	  * @param fieldName name of the field for which to create value
	  * @return a new value object for the field
	  */
	ref <headerFieldValue> createValue(const string& fieldName);
};


} // vmime


#endif // VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
