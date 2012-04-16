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

#ifndef VMIME_COMPONENT_HPP_INCLUDED
#define VMIME_COMPONENT_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/utility/inputStream.hpp"
#include "vmime/utility/seekableInputStream.hpp"
#include "vmime/utility/parserInputStreamAdapter.hpp"
#include "vmime/utility/outputStream.hpp"


namespace vmime
{


/** This abstract class is the base class for all the components of a message.
  * It defines methods for parsing and generating a component.
  */

class component : public object
{
public:

	component();
	virtual ~component();

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param buffer input buffer
	  */
	void parse(const string& buffer);

	/** Parse RFC-822/MIME data for this component. If stream is not seekable,
	  * or if length is not specified, entire contents of the stream will
	  * be loaded into memory before parsing.
	  */
	void parse(ref <utility::inputStream> inputStream, const utility::stream::size_type length);

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param buffer input buffer
	  * @param position current position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  */
	void parse
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	/** Parse RFC-822/MIME data for this component. If stream is not seekable,
	  * or if end position is not specified, entire contents of the stream will
	  * be loaded into memory before parsing.
	  *
	  * @param inputStream stream from which to read data
	  * @param position current position in the input stream
	  * @param end end position in the input stream
	  * @param newPosition will receive the new position in the input stream
	  */
	void parse
		(ref <utility::inputStream> inputStream,
		 const utility::stream::size_type position,
		 const utility::stream::size_type end,
		 utility::stream::size_type* newPosition = NULL);

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * \deprecated Use the new generate() method, which takes an outputStream parameter.
	  *
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @return generated data
	  */
	virtual const string generate
		(const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0) const;

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * @param outputStream output stream
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate
		(utility::outputStream& outputStream,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * @param outputStream output stream
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate
		(ref <utility::outputStream> outputStream,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;

	/** Clone this component.
	  *
	  * @return a copy of this component
	  */
	virtual ref <component> clone() const = 0;

	/** Replace data in this component by data in other component.
	  * Both components must be of the same type.
	  *
	  * @throw std::bad_cast_exception if the components are not
	  * of the same (dynamic) type
	  * @param other other component to copy data from
	  */
	virtual void copyFrom(const component& other) = 0;

	/** Return the start position of this component in the
	  * parsed message contents. Use for debugging only.
	  *
	  * @return start position in parsed buffer
	  * or 0 if this component has not been parsed
	  */
	utility::stream::size_type getParsedOffset() const;

	/** Return the length of this component in the
	  * parsed message contents. Use for debugging only.
	  *
	  * @return length of the component in parsed buffer
	  * or 0 if this component has not been parsed
	  */
	utility::stream::size_type getParsedLength() const;

	/** Return the list of children of this component.
	  *
	  * @return list of child components
	  */
	virtual const std::vector <ref <component> > getChildComponents() = 0;

protected:

	void setParsedBounds(const utility::stream::size_type start, const utility::stream::size_type end);

	// AT LEAST ONE of these parseImpl() functions MUST be implemented in derived class
	virtual void parseImpl
		(ref <utility::parserInputStreamAdapter> parser,
		 const utility::stream::size_type position,
		 const utility::stream::size_type end,
		 utility::stream::size_type* newPosition = NULL);

	virtual void parseImpl
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	virtual void generateImpl
		(utility::outputStream& os,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const = 0;

private:

	void offsetParsedBounds(const utility::stream::size_type offset);

	utility::stream::size_type m_parsedOffset;
	utility::stream::size_type m_parsedLength;
};


} // vmime


#endif // VMIME_COMPONENT_HPP_INCLUDED
