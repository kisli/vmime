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

#ifndef VMIME_CHARSET_HPP_INCLUDED
#define VMIME_CHARSET_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/utility/inputStream.hpp"
#include "vmime/utility/outputStream.hpp"
#include "vmime/component.hpp"


namespace vmime
{


class encoding;  // forward reference


/** Charset description (basic type).
  */

class charset : public component
{
public:

	charset();
	charset(const string& name);
	charset(const char* name); // to allow creation from vmime::charsets constants

public:

	/** Return the ISO name of the charset.
	  *
	  * @return charset name
	  */
	const string& getName() const;

	charset& operator=(const charset& other);

	bool operator==(const charset& value) const;
	bool operator!=(const charset& value) const;

	const std::vector <ref <component> > getChildComponents();

	/** Gets the recommended encoding for this charset.
	  * Note: there may be no recommended encoding.
	  *
	  * @param enc output parameter that will hold recommended encoding
	  * @return true if an encoding is recommended (the encoding is stored
	  * in the enc parameter), false otherwise (in this case, the enc
	  * parameter is not modified)
	  */
	bool getRecommendedEncoding(encoding& enc) const;

	/** Returns the default charset used on the system.
	  *
	  * This function simply calls <code>platformHandler::getLocaleCharset()</code>
	  * and is provided for convenience.
	  *
	  * @return system default charset
	  */
	static const charset getLocaleCharset();

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
	  * @throws exceptions::charset_conv_error if an error occured during
	  * the conversion
	  */
	static void convert(const string& in, string& out, const charset& source, const charset& dest);

	/** Convert the contents of an input stream in a specified charset
	  * to another charset and write the result to an output stream.
	  *
	  * @param in input stream to read data from
	  * @param out output stream to write the converted data
	  * @param source input charset
	  * @param dest output charset
	  * @throws exceptions::charset_conv_error if an error occured during
	  * the conversion
	  */
	static void convert(utility::inputStream& in, utility::outputStream& out, const charset& source, const charset& dest);

	ref <component> clone() const;
	void copyFrom(const component& other);

private:

	string m_name;

protected:

	// Component parsing & assembling
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
};


} // vmime


#endif // VMIME_CHARSET_HPP_INCLUDED
