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

#include "vmime/config.hpp"

#include "vmime/utility/idna.hpp"
#include "vmime/utility/stringUtils.hpp"


#ifndef VMIME_BUILDING_DOC

#if VMIME_IDNA_LIB_IS_LIBIDN2
#	include <idn2.h>
#elif VMIME_IDNA_LIB_IS_ICU
#	include <unicode/uidna.h>
#endif

#endif // VMIME_BUILDING_DOC


#include <limits>
#include <memory>
#include <vector>


namespace vmime {
namespace utility {


#if VMIME_IDNA_LIB_IS_LIBIDN2


// UTS #46 non-transitional processing, then the checks of IDNA2008. Not with
// IDN2_USE_STD3_ASCII_RULES: libidn2 then removes the characters which are not
// allowed in host names, instead of failing ("bü@cher" gives "xn--bcher-kva").
static const int LIBIDN2_FLAGS = IDN2_NONTRANSITIONAL;


static bool libraryToASCII(const string& domain, string& asciiDomain) {

	char* out = NULL;

	if (idn2_to_ascii_8z(domain.c_str(), &out, LIBIDN2_FLAGS) != IDN2_OK) {
		return false;
	}

	asciiDomain = out;
	idn2_free(out);

	return true;
}


// Only decodes Punycode: the resulting U-label is not checked
static bool decodeALabel(const string& aLabel, string& uLabel) {

	char* out = NULL;

	if (idn2_to_unicode_8z8z(aLabel.c_str(), &out, 0) != IDN2_OK) {
		return false;
	}

	uLabel = out;
	idn2_free(out);

	return true;
}


#elif VMIME_IDNA_LIB_IS_ICU


// UTS #46 non-transitional processing, with the checks of IDNA2008
static const uint32_t ICU_IDNA_OPTIONS =
	UIDNA_USE_STD3_RULES |
	UIDNA_CHECK_BIDI |
	UIDNA_CHECK_CONTEXTJ |
	UIDNA_CHECK_CONTEXTO |
	UIDNA_NONTRANSITIONAL_TO_ASCII |
	UIDNA_NONTRANSITIONAL_TO_UNICODE;


typedef int32_t (*ICUIDNAFunction)(
	const UIDNA* idna,
	const char* in, int32_t inLength,
	char* out, int32_t outCapacity,
	UIDNAInfo* info, UErrorCode* err
);


// Calls one of the UTF-8 functions of ICU; fails if the input is not valid
static bool callICU(const ICUIDNAFunction func, const string& in, string& out) {

	if (in.length() > static_cast <size_t>(std::numeric_limits <int32_t>::max())) {
		return false;
	}

	UErrorCode err = U_ZERO_ERROR;
	std::unique_ptr <UIDNA, void (*)(UIDNA*)> idna(
		uidna_openUTS46(ICU_IDNA_OPTIONS, &err), [](UIDNA* p) { uidna_close(p); }
	);

	if (U_FAILURE(err)) {
		return false;
	}

	std::vector <char> buffer(256);

	// Again with a larger buffer, if the first one was too small
	for (int pass = 0 ; pass < 2 ; ++pass) {

		UIDNAInfo info = UIDNA_INFO_INITIALIZER;
		err = U_ZERO_ERROR;

		const int32_t length = func(
			idna.get(),
			in.data(), static_cast <int32_t>(in.length()),
			&buffer[0], static_cast <int32_t>(buffer.size()),
			&info, &err
		);

		if (err == U_BUFFER_OVERFLOW_ERROR) {
			buffer.resize(length);
			continue;
		}

		if (U_FAILURE(err) || info.errors != 0) {
			return false;
		}

		out.assign(&buffer[0], length);
		return true;
	}

	return false;
}


static bool libraryToASCII(const string& domain, string& asciiDomain) {

	return callICU(uidna_nameToASCII_UTF8, domain, asciiDomain);
}


// Decodes Punycode, and also checks the resulting U-label
static bool decodeALabel(const string& aLabel, string& uLabel) {

	return callICU(uidna_labelToUnicodeUTF8, aLabel, uLabel);
}


#else // no IDNA library


static bool libraryToASCII(const string& /* domain */, string& /* asciiDomain */) {

	return false;
}


static bool decodeALabel(const string& /* aLabel */, string& /* uLabel */) {

	return false;
}


#endif // VMIME_IDNA_LIB_IS_xxx


static bool convertToASCII(const string& domain, string& asciiDomain) {

	string out;

	if (!libraryToASCII(domain, out)) {
		return false;
	}

	// Domain names in email are host names, only made of letters, digits
	// and hyphens (RFC 5321, section 4.1.2), as are A-labels
	for (const char c : out) {

		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		      (c >= '0' && c <= '9') || c == '-' || c == '.')) {

			return false;
		}
	}

	asciiDomain = out;

	return true;
}


// Converts a label to a U-label, if it is a valid A-label
static string labelToUnicode(const string& label) {

	// The "xn--" prefix is case-insensitive (RFC 5891, section 5.3), and
	// a label is at most 63 octets long (RFC 1035, section 2.3.4)
	if (label.length() <= 4 || label.length() > 63 ||
	    !stringUtils::isStringEqualNoCase(label, "xn--", 4) ||
	    !stringUtils::is7bit(label) || label.find('\0') != string::npos) {

		return label;
	}

	// The A-label is converted to lowercase before it is decoded, and the
	// resulting U-label must encode back to the same A-label (RFC 5891,
	// section 5.3): this also checks the U-label, if the library did not
	const string aLabel = stringUtils::toLower(label);
	string uLabel, check;

	if (!decodeALabel(aLabel, uLabel) ||
	    !convertToASCII(uLabel, check) || check != aLabel) {

		return label;
	}

	return uLabel;
}


// static
bool idna::isSupported() {

	return VMIME_IDNA_LIB_IS_LIBIDN2 || VMIME_IDNA_LIB_IS_ICU;
}


// static
bool idna::toASCII(const string& domain, string& asciiDomain) {

	if (stringUtils::is7bit(domain)) {

		asciiDomain = domain;
		return true;
	}

	return domain.find('\0') == string::npos && convertToASCII(domain, asciiDomain);
}


// static
string idna::toUnicode(const string& domain) {

	if (!isSupported()) {
		return domain;
	}

	string out;
	out.reserve(domain.length());

	for (size_t pos = 0 ; ; ) {

		const size_t dot = domain.find('.', pos);
		const size_t end = (dot == string::npos ? domain.length() : dot);

		out += labelToUnicode(string(domain.begin() + pos, domain.begin() + end));

		if (dot == string::npos) {
			break;
		}

		out += '.';
		pos = dot + 1;
	}

	return out;
}


} // utility
} // vmime
