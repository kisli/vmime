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

#ifndef VMIME_CHARSET_HPP_INCLUDED
#define VMIME_CHARSET_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Charset description (basic type).
  */

class charset : public component
{
public:

	charset();
	charset(const string& name);

public:

	/** Return the ISO name of the charset.
	  *
	  * @return charset name
	  */
	const string& getName() const;

	charset& operator=(const charset& other);
	charset& operator=(const string& name);

	const bool operator==(const charset& value) const;
	const bool operator!=(const charset& value) const;

	const std::vector <const component*> getChildComponents() const;

	/** Returns the default charset used on the system.
	  *
	  * This function simply calls <code>platformDependantHandler::getLocaleCharset()</code>
	  * and is provided for convenience.
	  *
	  * @return system default charset
	  */
	static const charset getLocaleCharset();

#if VMIME_WIDE_CHAR_SUPPORT
	/** Convert a string buffer in the specified charset to a wide-char
	  * string buffer.
	  *
	  * @param in input buffer
	  * @param out output buffer
	  * @param ch input charset
	  */
	static void decode(const string& in, wstring& out, const charset& ch);

	/** Convert a wide-char string buffer to a string buffer in the
	  * specified charset.
	  *
	  * @param in input buffer
	  * @param out output buffer
	  * @param ch output charset
	  */
	static void encode(const wstring& in, string& out, const charset& ch);
#endif

	/** Convert a string buffer from one charset to another
	  * charset (in-memory conversion)
	  *
	  * \deprecated Use the new convert() method, which takes
	  * an outputStream parameter.
	  *
	  * @param in input buffer
	  * @param out output buffer
	  * @param source input charset
	  * @param dest output charset
	  */
	static void convert(const string& in, string& out, const charset& source, const charset& dest);

	/** Convert the contents of an input stream in a specified charset
	  * to another charset and write the result to an output stream.
	  *
	  * @param in input stream to read data from
	  * @param out output stream to write the converted data
	  * @param source input charset
	  * @param dest output charset
	  */
	static void convert(utility::inputStream& in, utility::outputStream& out, const charset& source, const charset& dest);

	charset* clone() const;
	void copyFrom(const component& other);

private:

	string m_name;

	template <class STRINGF, class STRINGT>
	static void iconvert(const STRINGF& in, STRINGT& out, const charset& from, const charset& to);

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_CHARSET_HPP_INCLUDED
