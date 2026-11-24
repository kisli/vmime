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

#ifndef VMIME_UTILITY_IDNA_HPP_INCLUDED
#define VMIME_UTILITY_IDNA_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/base.hpp"


namespace vmime {
namespace utility {


/** Conversion of internationalized domain names, as specified by IDNA2008
  * (RFC 5890 to 5893). VMime relies on GNU Libidn2 or ICU for this, depending
  * on the build options.
  */
class VMIME_EXPORT idna {

public:

	/** Tells whether VMime was built with support for internationalized
	  * domain names. If not, toASCII() fails on any domain name which is not
	  * only made of ASCII characters, and toUnicode() returns its input
	  * unchanged.
	  *
	  * @return true if internationalized domain names are supported,
	  * false otherwise
	  */
	static bool isSupported();

	/** Converts a domain name to its ASCII form, in which each non-ASCII label
	  * is written as an A-label ("bücher.example" gives "xn--bcher-kva.example").
	  *
	  * The domain name is first mapped as specified by UTS #46 (non-transitional
	  * processing), so that "BÜCHER.example" gives the same result, then checked
	  * against the rules of IDNA2008 (RFC 5891, section 5.4). Its labels must
	  * also be host names, only made of letters, digits and hyphens once
	  * converted (RFC 5321, section 4.1.2).
	  *
	  * A domain name which is only made of ASCII characters is returned
	  * unchanged, without any check.
	  *
	  * @param domain domain name, in UTF-8
	  * @param asciiDomain on success, receives the domain name in ASCII
	  * @return true on success, or false if the domain name is not a valid
	  * internationalized domain name, or if internationalized domain names
	  * are not supported
	  */
	static bool toASCII(const string& domain, string& asciiDomain);

	/** Converts the A-labels of a domain name to U-labels ("xn--bcher-kva.example"
	  * gives "bücher.example"), for display.
	  *
	  * A label is only converted if it is a valid A-label: it decodes to a valid
	  * U-label, which encodes back to the same A-label (RFC 5891, section 5.3).
	  * Other labels are returned unchanged: so, a label which looks like an
	  * A-label cannot be displayed as something else than what it really is.
	  *
	  * @param domain domain name, in ASCII or UTF-8
	  * @return domain name, in UTF-8
	  */
	static string toUnicode(const string& domain);
};


} // utility
} // vmime


#endif // VMIME_UTILITY_IDNA_HPP_INCLUDED
