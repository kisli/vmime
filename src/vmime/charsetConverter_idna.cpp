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

#include "vmime/charsetConverter_idna.hpp"

#include "vmime/exception.hpp"

#include "vmime/utility/idna.hpp"
#include "vmime/utility/streamUtils.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"

#include "contrib/utf8/utf8.h"


namespace vmime {


charsetConverter_idna::charsetConverter_idna(
	const charset& source,
	const charset& dest,
	const charsetConverterOptions& opts
)
	: m_source(source),
	  m_dest(dest),
	  m_options(opts) {

}


charsetConverter_idna::~charsetConverter_idna() {

}


void charsetConverter_idna::convert(utility::inputStream& in, utility::outputStream& out, status* st) {

	if (st) {
		new (st) status();
	}

	// IDNA should be used for short strings, so it does not matter if we
	// do not work directly on the stream
	string inStr;
	vmime::utility::outputStreamStringAdapter os(inStr);
	vmime::utility::bufferedStreamCopy(in, os);

	string outStr;
	convert(inStr, outStr, st);

	out << outStr;
}


void charsetConverter_idna::convert(const string& in, string& out, status* st) {

	if (st) {
		new (st) status();
	}

	out.clear();

	if (m_dest == "idna") {

		string inUTF8;
		charset::convert(in, inUTF8, m_source, vmime::charsets::UTF_8, m_options);

		// Invalid sequences are not replaced here: the result would not be
		// a valid domain name either
		if (!utf8::is_valid(inUTF8.begin(), inUTF8.end())) {
			throw exceptions::illegal_byte_sequence_for_charset();
		}

		if (!utility::idna::toASCII(inUTF8, out)) {

			throw exceptions::charset_conv_error(
				utility::idna::isSupported()
					? "Not a valid internationalized domain name."
					: "Internationalized domain names are not supported."
			);
		}

	} else if (m_source == "idna") {

		charset::convert(utility::idna::toUnicode(in), out, vmime::charsets::UTF_8, m_dest, m_options);
	}

	if (st) {
		st->inputBytesRead = in.length();
		st->outputBytesWritten = out.length();
	}
}


shared_ptr <utility::charsetFilteredOutputStream>
	charsetConverter_idna::getFilteredOutputStream(
		utility::outputStream& /* os */,
		const charsetConverterOptions& /* opts */
	) {

	// Not supported
	return null;
}


} // vmime
