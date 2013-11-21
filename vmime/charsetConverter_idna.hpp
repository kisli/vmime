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

#ifndef VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED
#define VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED


#include "vmime/charsetConverter.hpp"


namespace vmime
{


/** A charset converter which can convert to and from Punycode (for IDNA).
  */

class charsetConverter_idna : public charsetConverter
{
public:

	/** Construct and initialize an IDNA charset converter.
	  *
	  * @param source input charset
	  * @param dest output charset
	  * @param opts conversion options
	  */
	charsetConverter_idna(const charset& source, const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions());

	~charsetConverter_idna();

	void convert(const string& in, string& out);
	void convert(utility::inputStream& in, utility::outputStream& out);

	shared_ptr <utility::charsetFilteredOutputStream> getFilteredOutputStream(utility::outputStream& os);

private:

	charset m_source;
	charset m_dest;

	charsetConverterOptions m_options;
};


} // vmime


#endif // VMIME_CHARSETCONVERTER_IDNA_HPP_INCLUDED
