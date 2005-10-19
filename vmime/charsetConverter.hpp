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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_CHARSETCONVERTER_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"

#include "vmime/charset.hpp"


namespace vmime
{


/** Convert between charsets.
  */

class charsetConverter : public object
{
public:

	/** Construct and initialize a charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  */
	charsetConverter(const charset& source, const charset& dest);

	~charsetConverter();

	/** Convert a string buffer from one charset to another
	  * charset (in-memory conversion)
	  *
	  * \deprecated Use the new convert() method, which takes
	  * an outputStream parameter.
	  *
	  * @param in input buffer
	  * @param out output buffer
	  * @throws exceptions::charset_conv_error if an error occured during
	  * the conversion
	  */
	void convert(const string& in, string& out);

	/** Convert the contents of an input stream in a specified charset
	  * to another charset and write the result to an output stream.
	  *
	  * @param in input stream to read data from
	  * @param out output stream to write the converted data
	  * @throws exceptions::charset_conv_error if an error occured during
	  * the conversion
	  */
	void convert(utility::inputStream& in, utility::outputStream& out);

private:

	void* m_desc;

	charset m_source;
	charset m_dest;
};


} // vmime


#endif // VMIME_CHARSETCONVERTER_HPP_INCLUDED

