//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED


#include "vmime/charsetConverter.hpp"


namespace vmime {


/** A pseudo charset converter for internationalized domain names (IDNA2008).
  *
  * To "idna", it converts a domain name to its ASCII form, in which non-ASCII
  * labels are written as A-labels. It throws exceptions::charset_conv_error
  * if the domain name is not valid, or if internationalized domain names are
  * not supported (see utility::idna::toASCII()).
  *
  * From "idna", it converts the valid A-labels of a domain name to U-labels,
  * and leaves other labels unchanged (see utility::idna::toUnicode()).
  */
class charsetConverter_idna : public charsetConverter {

public:

	/** Construct and initialize an IDNA charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	charsetConverter_idna(
		const charset& source,
		const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions()
	);

	~charsetConverter_idna();

	void convert(const string& in, string& out, status* st = NULL);
	void convert(utility::inputStream& in, utility::outputStream& out, status* st = NULL);

	shared_ptr <utility::charsetFilteredOutputStream> getFilteredOutputStream(
		utility::outputStream& os,
		const charsetConverterOptions& opts = charsetConverterOptions()
	);

private:

	charset m_source;
	charset m_dest;

	charsetConverterOptions m_options;
};


} // vmime


#endif // VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED
