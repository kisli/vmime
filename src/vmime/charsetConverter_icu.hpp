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

#ifndef VMIME_CHARSETCONVERTER_ICU_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_ICU_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_ICU


#include "vmime/charsetConverter.hpp"


struct UConverter;


namespace vmime
{


/** A generic charset converter which uses ICU library.
  */

class charsetConverter_icu : public charsetConverter
{
public:

	/** Construct and initialize an ICU charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	charsetConverter_icu(const charset& source, const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions());

	~charsetConverter_icu();

	void convert(const string& in, string& out);
	void convert(utility::inputStream& in, utility::outputStream& out);

	shared_ptr <utility::charsetFilteredOutputStream> getFilteredOutputStream(utility::outputStream& os);

private:

	UConverter* m_from;
	UConverter* m_to;

	charset m_source;
	charset m_dest;

	charsetConverterOptions m_options;
};


namespace utility {


class charsetFilteredOutputStream_icu : public charsetFilteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param os stream into which write filtered data
	  */
	charsetFilteredOutputStream_icu
		(const charset& source, const charset& dest, outputStream* os);

	~charsetFilteredOutputStream_icu();


	outputStream& getNextOutputStream();

	void flush();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

	UConverter* m_from;
	UConverter* m_to;

	const charset m_sourceCharset;
	const charset m_destCharset;

	outputStream& m_stream;
};


} // utility


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_ICU

#endif // VMIME_CHARSETCONVERTER_ICU_HPP_INCLUDED
