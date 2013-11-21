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

#ifndef VMIME_CHARSETCONVERTER_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"

#include "vmime/charset.hpp"
#include "vmime/charsetConverterOptions.hpp"
#include "vmime/utility/filteredStream.hpp"


namespace vmime
{


namespace utility
{


/** A filtered output stream which applies a charset conversion
  * to input bytes.
  *
  * May throw a exceptions::charset_conv_error if an error
  * occured when initializing convert, or during charset conversion.
  */

class VMIME_EXPORT charsetFilteredOutputStream : public filteredOutputStream
{
};


} // utility


/** Convert between charsets.
  */

class VMIME_EXPORT charsetConverter : public object
{
public:

	/** Construct and initialize an iconv charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	static shared_ptr <charsetConverter> create
		(const charset& source, const charset& dest,
		 const charsetConverterOptions& opts = charsetConverterOptions());

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
	virtual void convert(const string& in, string& out) = 0;

	/** Convert the contents of an input stream in a specified charset
	  * to another charset and write the result to an output stream.
	  *
	  * @param in input stream to read data from
	  * @param out output stream to write the converted data
	  * @throws exceptions::charset_conv_error if an error occured during
	  * the conversion
	  */
	virtual void convert(utility::inputStream& in, utility::outputStream& out) = 0;

	/** Returns a filtered output stream which applies a charset
	  * conversion to input bytes. Please note that it may not be
	  * supported by the converter.
	  *
	  * @param os stream into which filtered data will be written
	  * @return a filtered output stream, or NULL if not supported
	  */
	virtual shared_ptr <utility::charsetFilteredOutputStream> getFilteredOutputStream(utility::outputStream& os) = 0;

private:

	static shared_ptr <charsetConverter> createGenericConverter
		(const charset& source, const charset& dest,
		 const charsetConverterOptions& opts);
};


} // vmime


#endif // VMIME_CHARSETCONVERTER_HPP_INCLUDED
