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

#include "vmime/charset.hpp"
#include "vmime/exception.hpp"
#include "vmime/platform.hpp"
#include "vmime/encoding.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/charsetConverter.hpp"



namespace vmime {


charset::charset()
	: m_name(charsets::US_ASCII) {

}


charset::charset(const string& name)
	: m_name(name) {

	// If we receive this rfc-1642 valid MIME charset, convert it to something usefull for iconv
	if (utility::stringUtils::isStringEqualNoCase(m_name, "unicode-1-1-utf-7")) {
		m_name = "utf-7";
	}
}


charset::charset(const char* name)
	: m_name(name) {

}


void charset::parseImpl(
	const parsingContext& /* ctx */,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition
) {

	m_name = utility::stringUtils::trim(
		string(buffer.begin() + position, buffer.begin() + end)
	);

	// If we parsed this rfc-1642 valid MIME charset, convert it to something usefull for iconv
	if (utility::stringUtils::isStringEqualNoCase(m_name, "unicode-1-1-utf-7")) {
		m_name = "utf-7";
	}

	setParsedBounds(position, end);

	if (newPosition) {
		*newPosition = end;
	}
}


void charset::generateImpl(
	const generationContext& /* ctx */,
	utility::outputStream& os,
	const size_t curLinePos,
	size_t* newLinePos
) const {

	os << m_name;

	if (newLinePos) {
		*newLinePos = curLinePos + m_name.length();
	}
}


void charset::convert(
	utility::inputStream& in,
	utility::outputStream& out,
	const charset& source,
	const charset& dest,
	const charsetConverterOptions& opts
) {

	shared_ptr <charsetConverter> conv = charsetConverter::create(source, dest, opts);
	conv->convert(in, out);
}


void charset::convert(
	const string& in,
	string& out,
	const charset& source,
	const charset& dest,
	const charsetConverterOptions& opts
) {

	if (source == dest) {
		out = in;
		return;
	}

	shared_ptr <charsetConverter> conv = charsetConverter::create(source, dest, opts);
	conv->convert(in, out);
}


bool charset::isValidText(const string& text, string::size_type* firstInvalidByte) const {

	charsetConverterOptions opts;
	opts.silentlyReplaceInvalidSequences = false;

	charsetConverter::status st;

	try {

		std::string out;

		// Try converting to UTF-8
		shared_ptr <charsetConverter> conv = charsetConverter::create(*this, vmime::charset("utf-8"), opts);
		conv->convert(text, out, &st);

	} catch (exceptions::illegal_byte_sequence_for_charset& e) {

		// An illegal byte sequence was found in the input buffer
		if (firstInvalidByte) {

			if (st.inputBytesRead < text.length())
				*firstInvalidByte = st.inputBytesRead;
			else
				*firstInvalidByte = text.length();
		}

		return false;
	}

	if (firstInvalidByte) {
		*firstInvalidByte = text.length();
	}

	return true;
}


const charset charset::getLocalCharset() {

	return platform::getHandler()->getLocalCharset();
}


charset& charset::operator=(const charset& other) {

	copyFrom(other);
	return *this;
}


bool charset::operator==(const charset& value) const {

	return utility::stringUtils::isStringEqualNoCase(m_name, value.m_name);
}


bool charset::operator!=(const charset& value) const {

	return !(*this == value);
}


shared_ptr <component> charset::clone() const {

	return make_shared <charset>(m_name);
}


const string& charset::getName() const {

	return m_name;
}


void charset::copyFrom(const component& other) {

	m_name = dynamic_cast <const charset&>(other).m_name;
}


const std::vector <shared_ptr <component> > charset::getChildComponents() {

	return std::vector <shared_ptr <component> >();
}



// Explicitly force encoding for some charsets
struct CharsetEncodingEntry {

	CharsetEncodingEntry(const string& charset_, const string& encoding_)
		: charset(charset_), encoding(encoding_) {

	}

	const string charset;
	const string encoding;
};


CharsetEncodingEntry g_charsetEncodingMap[] = {

	// Use QP encoding for ISO-8859-x charsets
	CharsetEncodingEntry("iso-8859",     encodingTypes::QUOTED_PRINTABLE),
	CharsetEncodingEntry("iso8859",      encodingTypes::QUOTED_PRINTABLE),

	// RFC-1468 states:
	//   " ISO-2022-JP may also be used in MIME Part 2 headers.  The "B"
	//     encoding should be used with ISO-2022-JP text. "
	// Use Base64 encoding for all ISO-2022 charsets.
	CharsetEncodingEntry("iso-2022",     encodingTypes::BASE64),
	CharsetEncodingEntry("iso2022",      encodingTypes::BASE64),

	// Last entry is not used
	CharsetEncodingEntry("", "")
};


bool charset::getRecommendedEncoding(encoding& enc) const {

	// Special treatment for some charsets
	const string cset = utility::stringUtils::toLower(getName());

	for (unsigned int i = 0 ;
	     i < (sizeof(g_charsetEncodingMap) / sizeof(g_charsetEncodingMap[0])) - 1 ;
	     ++i) {

		if (cset.find(g_charsetEncodingMap[i].charset) != string::npos) {
			enc = g_charsetEncodingMap[i].encoding;
			return true;
		}
	}

	return false;
}


} // vmime
