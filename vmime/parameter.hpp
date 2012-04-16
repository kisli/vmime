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

#ifndef VMIME_PARAMETER_HPP_INCLUDED
#define VMIME_PARAMETER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/word.hpp"


namespace vmime
{


class parameter : public component
{
	friend class parameterizedHeaderField;

private:

	parameter(const parameter&);

public:

	parameter(const string& name);
	parameter(const string& name, const word& value);
	parameter(const string& name, const string& value);


#ifndef VMIME_BUILDING_DOC

	/** A single section of a multi-section parameter,
	  * as defined in RFC-2231/3. This is used when
	  * calling parse() on the parameter.
	  */
	struct valueChunk
	{
		bool encoded;
		string data;
	};

#endif // VMIME_BUILDING_DOC

	ref <component> clone() const;
	void copyFrom(const component& other);
	parameter& operator=(const parameter& other);

	const std::vector <ref <component> > getChildComponents();

	/** Return the name of this parameter.
	  *
	  * @return name of this parameter
	  */
	const string& getName() const;

	/** Return the raw value of this parameter.
	  *
	  * @return read-only value
	  */
	const word& getValue() const;

	/** Return the value of this object in the specified type.
	  * For example, the following code:
	  *
	  * <pre>
	  *    getParameter("creation-date")->getValueAs <vmime::dateTime>()
	  * </pre>
	  *
	  * is equivalent to:
	  *
	  * <pre>
	  *    ref <vmime::word> rawValue = getParameter("creation-date");
	  *
	  *    vmime::dateTime theDate;
	  *    theDate.parse(rawValue->getBuffer());
	  * </pre>
	  *
	  * @param T type to which convert the value
	  * @return value
	  */
	template <typename T>
	const T getValueAs() const
	{
		T ret;
		ret.parse(m_value->getBuffer());

		return ret;
	}

	/** Set the value of this parameter.
	  *
	  * @param value new value
	  */
	void setValue(const component& value);

	/** Set the raw value of this parameter.
	  *
	  * @param value new value
	  */
	void setValue(const word& value);


protected:

	void parseImpl
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	void generateImpl
		(utility::outputStream& os,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;

private:

	void parse(const std::vector <valueChunk>& chunks);


	string m_name;
	ref <word> m_value;
};


} // vmime


#endif // VMIME_PARAMETER_HPP_INCLUDED
