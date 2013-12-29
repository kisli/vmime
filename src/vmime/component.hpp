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

#ifndef VMIME_COMPONENT_HPP_INCLUDED
#define VMIME_COMPONENT_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/utility/inputStream.hpp"
#include "vmime/utility/seekableInputStream.hpp"
#include "vmime/utility/parserInputStreamAdapter.hpp"
#include "vmime/utility/outputStream.hpp"
#include "vmime/generationContext.hpp"
#include "vmime/parsingContext.hpp"


namespace vmime
{


/** This abstract class is the base class for all the components of a message.
  * It defines methods for parsing and generating a component.
  */

class VMIME_EXPORT component : public object
{
public:

	component();
	virtual ~component();

	/** Parse RFC-822/MIME data for this component, using the default
	  * parsing context.
	  *
	  * @param buffer input buffer
	  */
	void parse(const string& buffer);

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param ctx parsing context
	  * @param buffer input buffer
	  */
	void parse(const parsingContext& ctx, const string& buffer);

	/** Parse RFC-822/MIME data for this component. If stream is not seekable,
	  * or if length is not specified, entire contents of the stream will
	  * be loaded into memory before parsing.
	  *
	  * @param inputStream stream from which to read data
	  * @param length data length, in bytes (0 = unknown/not specified)
	  */
	void parse(shared_ptr <utility::inputStream> inputStream, const size_t length);

	/** Parse RFC-822/MIME data for this component, using the default
	  * parsing context.
	  *
	  * @param buffer input buffer
	  * @param position current position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  */
	void parse
		(const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	/** Parse RFC-822/MIME data for this component.
	  *
	  * @param ctx parsing context
	  * @param buffer input buffer
	  * @param position current position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  */
	void parse
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	/** Parse RFC-822/MIME data for this component. If stream is not seekable,
	  * or if end position is not specified, entire contents of the stream will
	  * be loaded into memory before parsing. The default parsing context
	  * will be used.
	  *
	  * @param inputStream stream from which to read data
	  * @param position current position in the input stream
	  * @param end end position in the input stream
	  * @param newPosition will receive the new position in the input stream
	  */
	void parse
		(shared_ptr <utility::inputStream> inputStream,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	/** Parse RFC-822/MIME data for this component. If stream is not seekable,
	  * or if end position is not specified, entire contents of the stream will
	  * be loaded into memory before parsing.
	  *
	  * @param ctx parsing context
	  * @param inputStream stream from which to read data
	  * @param position current position in the input stream
	  * @param end end position in the input stream
	  * @param newPosition will receive the new position in the input stream
	  */
	void parse
		(const parsingContext& ctx,
		 shared_ptr <utility::inputStream> inputStream,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	/** Generate RFC-2822/MIME data for this component.
	  *
	  * \deprecated Use the new generate() method, which takes an outputStream parameter.
	  *
	  * @param maxLineLength maximum line length for output
	  * @param curLinePos length of the current line in the output buffer
	  * @return generated data
	  */
	virtual const string generate
		(const size_t maxLineLength = lineLengthLimits::infinite,
		 const size_t curLinePos = 0) const;

	/** Generate RFC-2822/MIME data for this component, using the default generation context.
	  *
	  * @param outputStream output stream
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate
		(utility::outputStream& outputStream,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;

	/** Generate RFC-2822/MIME data for this component, using the default generation context.
	  *
	  * @param ctx generation context
	  * @param outputStream output stream
	  * @param curLinePos length of the current line in the output buffer
	  * @param newLinePos will receive the new line position (length of the last line written)
	  */
	virtual void generate
		(const generationContext& ctx,
		 utility::outputStream& outputStream,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;

	/** Clone this component.
	  *
	  * @return a copy of this component
	  */
	virtual shared_ptr <component> clone() const = 0;

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
	size_t getParsedOffset() const;

	/** Return the length of this component in the
	  * parsed message contents. Use for debugging only.
	  *
	  * @return length of the component in parsed buffer
	  * or 0 if this component has not been parsed
	  */
	size_t getParsedLength() const;

	/** Return the list of children of this component.
	  *
	  * @return list of child components
	  */
	virtual const std::vector <shared_ptr <component> > getChildComponents() = 0;

	/** Get the number of bytes that will be used by this component when
	  * it is generated. This may be a heuristically-derived estimate,
	  * but such an estimated size should always be larger than the actual
	  * generated size.
	  *
	  * @param ctx generation context
	  * @return component size when generated
	  */
	virtual size_t getGeneratedSize(const generationContext& ctx);

protected:

	void setParsedBounds(const size_t start, const size_t end);

	// AT LEAST ONE of these parseImpl() functions MUST be implemented in derived class
	virtual void parseImpl
		(const parsingContext& ctx,
		 shared_ptr <utility::parserInputStreamAdapter> parser,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	virtual void parseImpl
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	virtual void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const = 0;

private:

	void offsetParsedBounds(const size_t offset);

	size_t m_parsedOffset;
	size_t m_parsedLength;
};


} // vmime


#endif // VMIME_COMPONENT_HPP_INCLUDED
