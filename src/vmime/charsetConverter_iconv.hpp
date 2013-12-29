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

#ifndef VMIME_CHARSETCONVERTER_ICONV_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_ICONV_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_ICONV


#include "vmime/charsetConverter.hpp"


namespace vmime
{


/** A generic charset converter which uses iconv library.
  */

class charsetConverter_iconv : public charsetConverter
{
public:

	/** Construct and initialize an iconv charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	charsetConverter_iconv(const charset& source, const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions());

	~charsetConverter_iconv();

	void convert(const string& in, string& out);
	void convert(utility::inputStream& in, utility::outputStream& out);

	shared_ptr <utility::charsetFilteredOutputStream> getFilteredOutputStream(utility::outputStream& os);

private:

	void* m_desc;

	charset m_source;
	charset m_dest;

	charsetConverterOptions m_options;
};


namespace utility {


class charsetFilteredOutputStream_iconv : public charsetFilteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param os stream into which write filtered data
	  */
	charsetFilteredOutputStream_iconv
		(const charset& source, const charset& dest, outputStream* os);

	~charsetFilteredOutputStream_iconv();


	outputStream& getNextOutputStream();

	void flush();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

	// Maximum character width in any charset
	enum { MAX_CHARACTER_WIDTH = 128 };


	void* m_desc;

	const charset m_sourceCharset;
	const charset m_destCharset;

	outputStream& m_stream;

	// Buffer in which unconverted bytes are left until they can
	// be converted (when more data arrives). The length should be
	// large enough to contain any character in any charset.
	byte_t m_unconvBuffer[MAX_CHARACTER_WIDTH];
	size_t m_unconvCount;

	// Buffer used for conversion. Avoids declaring it in write().
	// Should be at least MAX_CHARACTER_WIDTH * MAX_CHARACTER_WIDTH.
	byte_t m_outputBuffer[32768];
};


} // utility


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_ICONV

#endif // VMIME_CHARSETCONVERTER_ICONV_HPP_INCLUDED
