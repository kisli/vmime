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


#include "base.hpp"


namespace vmime
{


/** This abstract class is the base for all the classes in the library.
  * It defines the methods for parsing and generating all the components.
  */

class component
{
protected:

	virtual ~component() {}

protected:

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

public:

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * \deprecated Use the new generate() method, which takes an outputStream parameter.
	  *
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @return generated data
	  */
	virtual const string generate(const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0) const;

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * @param os output stream
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const = 0;
};


} // vmime


#endif // VMIME_COMPONENT_HPP_INCLUDED
