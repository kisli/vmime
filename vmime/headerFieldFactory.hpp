//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

class VMIME_EXPORT headerFieldFactory
{
protected:

	headerFieldFactory();
	~headerFieldFactory();

	typedef shared_ptr <headerField> (*AllocFunc)(void);
	typedef std::map <string, AllocFunc> NameMap;

	NameMap m_nameMap;


	struct ValueInfo
	{
		typedef shared_ptr <headerFieldValue> (*ValueAllocFunc)(void);
		typedef bool (*ValueTypeCheckFunc)(const object&);

		ValueAllocFunc allocFunc;
		ValueTypeCheckFunc checkTypeFunc;
	};

	typedef std::map <string, ValueInfo> ValueMap;

	ValueMap m_valueMap;

public:

	static shared_ptr <headerFieldFactory> getInstance();

#ifndef VMIME_BUILDING_DOC
	// TYPE must inherit from BASE_TYPE
	template <class BASE_TYPE, class TYPE>
	class registerer
	{
	public:

		static bool checkType(const object& obj)
		{
			const TYPE* typedObj = dynamic_cast <const TYPE*>(&obj);
			return typedObj != NULL;
		}

		static shared_ptr <BASE_TYPE> creator()
		{
			// Allocate a new object
			return shared_ptr <BASE_TYPE>(new TYPE());
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
		ValueInfo vi;
		vi.allocFunc = &registerer <headerFieldValue, T>::creator;
		vi.checkTypeFunc = &registerer <headerField, T>::checkType;

		m_valueMap.insert(ValueMap::value_type
			(utility::stringUtils::toLower(name), vi));
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
	shared_ptr <headerField> create(const string& name, const string& body = NULL_STRING);

	/** Create a new field value for the specified field.
	  *
	  * @param fieldName name of the field for which to create value
	  * @return a new value object for the field
	  */
	shared_ptr <headerFieldValue> createValue(const string& fieldName);

	/** Returns whether the specified value type is valid for the specified field.
	  *
	  * @param field header field
	  * @param value value for this header field
	  * @return true if the value type is compatible with the header field, or
	  * false otherwise
	  */
	bool isValueTypeValid(const headerField& field, const headerFieldValue& value) const;
};


} // vmime


#endif // VMIME_HEADERFIELDFACTORY_HPP_INCLUDED
