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

#ifndef VMIME_COMPONENT_HPP_INCLUDED
#define VMIME_COMPONENT_HPP_INCLUDED


#include "vmime/base.hpp"


namespace vmime
{


/** This abstract class is the base for all the classes in the library.
  * It defines the methods for parsing and generating all the components.
  */

class component
{
public:

	component();
	virtual ~component();

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param buffer input buffer
	  */
	void parse(const string& buffer);

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param buffer input buffer
	  * @param position current position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  */
	virtual void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL) = 0;

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * \deprecated Use the new generate() method, which takes an outputStream parameter.
	  *
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @return generated data
	  */
	const string generate(const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0) const;

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * @param os output stream
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const = 0;

	/** Clone this component.
	  *
	  * @return a copy of this component
	  */
	virtual component* clone() const = 0;

	/** Replace data in this component by data in other component.
	  * Both components must be of the same type.
	  *
	  * @throw std::bad_cast_exception if the components are not
	  * of the same (dynamic) type
	  * @param other other component to copy data from
	  */
	virtual void copyFrom(const component& other) = 0;

	/** Return the start position of this component in the
	  * parsed message contents.
	  *
	  * @return start position in parsed buffer
	  * or 0 if this component has not been parsed
	  */
	const string::size_type getParsedOffset() const;

	/** Return the length of this component in the
	  * parsed message contents.
	  *
	  * @return length of the component in parsed buffer
	  * or 0 if this component has not been parsed
	  */
	const string::size_type getParsedLength() const;

	/** Return the list of children of this component.
	  *
	  * @return list of child components
	  */
	const std::vector <component*> getChildComponents();

	/** Return the list of children of this component (const version).
	  *
	  * @return list of child components
	  */
	virtual const std::vector <const component*> getChildComponents() const = 0;

protected:

	void setParsedBounds(const string::size_type start, const string::size_type end);

private:

	string::size_type m_parsedOffset;
	string::size_type m_parsedLength;
};


} // vmime


#endif // VMIME_COMPONENT_HPP_INCLUDED
