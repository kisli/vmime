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

#ifndef VMIME_CHARSETCONVERTER_WIN_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_WIN_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_WIN


#include "vmime/charsetConverter.hpp"


namespace vmime
{


/** A generic charset converter which uses Windows MultiByteToWideChar.
  */

class charsetConverter_win : public charsetConverter
{
public:

	/** Construct and initialize a Windows charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	charsetConverter_win(const charset& source, const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions());

	void convert(const string& in, string& out);
	void convert(utility::inputStream& in, utility::outputStream& out);

	shared_ptr <utility::charsetFilteredOutputStream>
		getFilteredOutputStream(utility::outputStream& os);

private:

	static int getCodePage(const char* name);

	charset m_source;
	charset m_dest;

	charsetConverterOptions m_options;
};


} // namespace


#endif // VMIME_CHARSETCONV_LIB_IS_WIN

#endif // VMIME_CHARSETCONVERTER_WIN_HPP_INCLUDED
