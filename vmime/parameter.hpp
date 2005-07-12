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

#ifndef VMIME_PARAMETER_HPP_INCLUDED
#define VMIME_PARAMETER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"


namespace vmime
{


class parameter : public component
{
	friend class parameterFactory;
	friend class parameterizedHeaderField;

public:

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

	const std::vector <ref <const component> > getChildComponents() const;

	/** Return the name of the parameter.
	  *
	  * @return name of the parameter
	  */
	const string& getName() const;

	/** Return the read-only value object attached to this field.
	  *
	  * @return read-only value object
	  */
	virtual const component& getValue() const = 0;

	/** Return the value object attached to this field.
	  *
	  * @return value object
	  */
	virtual component& getValue() = 0;

	/** Set the value of the parameter.
	  *
	  * @throw std::bad_cast_exception if the value type is
	  * incompatible with the parameter type
	  * @param value value object
	  */
	virtual void setValue(const component& value) = 0;

	using component::parse;
	using component::generate;

	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;

protected:

	virtual const ref <const component> getValueImp() const = 0;
	virtual const ref <component> getValueImp() = 0;

private:

	string m_name;

	void generateValue(utility::outputStream& os, const string::size_type maxLineLength, const string::size_type curLinePos, string::size_type* newLinePos) const;

	virtual void parse(const std::vector <valueChunk>& chunks);
};


} // vmime


#endif // VMIME_PARAMETER_HPP_INCLUDED
