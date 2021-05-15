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


#if VMIME_CHARSETCONV_LIB_IS_ICU


#include "vmime/charsetConverter_icu.hpp"

#include "vmime/exception.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


#ifndef VMIME_BUILDING_DOC

	#include <unicode/ucnv.h>
	#include <unicode/ucnv_err.h>

#endif // VMIME_BUILDING_DOC


#include <unicode/unistr.h>


namespace vmime {


// static
shared_ptr <charsetConverter> charsetConverter::createGenericConverter(
	const charset& source,
	const charset& dest,
	const charsetConverterOptions& opts
) {

	return make_shared <charsetConverter_icu>(source, dest, opts);
}


charsetConverter_icu::charsetConverter_icu(
	const charset& source,
	const charset& dest,
	const charsetConverterOptions& opts
)
	: m_from(NULL),
	  m_to(NULL),
	  m_source(source),
	  m_dest(dest),
	  m_options(opts) {

	UErrorCode err = U_ZERO_ERROR;
	m_from = ucnv_open(source.getName().c_str(), &err);

	if (!U_SUCCESS(err)) {

		throw exceptions::charset_conv_error(
			"Cannot initialize ICU converter for source charset '" + source.getName()
			+ "' (error code: " + u_errorName(err) + "."
		);
	}

	m_to = ucnv_open(dest.getName().c_str(), &err);

	if (!U_SUCCESS(err)) {

		throw exceptions::charset_conv_error(
			"Cannot initialize ICU converter for destination charset '" + dest.getName()
			+ "' (error code: " + u_errorName(err) + "."
		);
	}
}


charsetConverter_icu::~charsetConverter_icu() {

	if (m_from) ucnv_close(m_from);
	if (m_to) ucnv_close(m_to);
}


void charsetConverter_icu::convert(
	utility::inputStream& in,
	utility::outputStream& out,
	status* st
) {

	UErrorCode err = U_ZERO_ERROR;

	ucnv_reset(m_from);
	ucnv_reset(m_to);

	if (st) {
		new (st) status();
	}

	// From buffers
	byte_t cpInBuffer[16]; // stream data put here
	const size_t outSize = ucnv_getMinCharSize(m_from) * sizeof(cpInBuffer) * sizeof(UChar);
	std::vector <UChar> uOutBuffer(outSize); // Unicode chars end up here

	// To buffers
	// converted (char) data end up here
	const size_t cpOutBufferSz = ucnv_getMaxCharSize(m_to) * outSize;
	std::vector <char> cpOutBuffer(cpOutBufferSz);

	// Tell ICU what to do when encountering an illegal byte sequence
	if (m_options.silentlyReplaceInvalidSequences) {

		// Set replacement chars for when converting from Unicode to codepage
		icu::UnicodeString substString(m_options.invalidSequence.c_str());
		ucnv_setSubstString(m_to, substString.getTerminatedBuffer(), -1, &err);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting substitution string.");
		}

	} else {

		// Tell ICU top stop (and return an error) on illegal byte sequences
		ucnv_setToUCallBack(
			m_from, UCNV_TO_U_CALLBACK_STOP, UCNV_SUB_STOP_ON_ILLEGAL, NULL, NULL, &err
		);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting ToU callback.");
		}

		ucnv_setFromUCallBack(
			m_to, UCNV_FROM_U_CALLBACK_STOP, UCNV_SUB_STOP_ON_ILLEGAL, NULL, NULL, &err
		);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting FromU callback.");
		}
	}

	// Input data available
	while (!in.eof()) {

		// Read input data into buffer
		size_t inLength = in.read(cpInBuffer, sizeof(cpInBuffer));

		// Beginning of read data
		const char* source = reinterpret_cast <const char*>(&cpInBuffer[0]);
		const char* sourceLimit = source + inLength; // end + 1

		UBool flush = in.eof();  // is this last run?

		UErrorCode toErr;

		// Loop until all source has been processed
		do {

			// Set up target pointers
			UChar* target = &uOutBuffer[0];
			UChar* targetLimit = &target[0] + outSize;

			toErr = U_ZERO_ERROR;

			ucnv_toUnicode(
				m_from, &target, targetLimit,
				&source, sourceLimit, NULL, flush, &toErr
			);

			if (st) {
				st->inputBytesRead += (source - reinterpret_cast <const char*>(&cpInBuffer[0]));
			}

			if (toErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(toErr)) {

				if (toErr == U_INVALID_CHAR_FOUND ||
				    toErr == U_TRUNCATED_CHAR_FOUND ||
				    toErr == U_ILLEGAL_CHAR_FOUND) {

					// Error will be thrown later (*)

				} else {

					throw exceptions::charset_conv_error(
						"[ICU] Error converting to Unicode from " + m_source.getName()
					);
				}
			}

			// The Unicode source is the buffer just written and the limit
			// is where the previous conversion stopped (target is moved in the conversion)
			const UChar* uSource = &uOutBuffer[0];
			UChar* uSourceLimit = &target[0];
			UErrorCode fromErr;

			// Loop until converted chars are fully written
			do {

				char* cpTarget = &cpOutBuffer[0];
				const char* cpTargetLimit = &cpOutBuffer[0] + cpOutBufferSz;

				fromErr = U_ZERO_ERROR;

				// Write converted bytes (Unicode) to destination codepage
				ucnv_fromUnicode(
					m_to, &cpTarget, cpTargetLimit,
					&uSource, uSourceLimit, NULL, flush, &fromErr
				);

				if (st) {

					// Decrement input bytes count by the number of input bytes in error
					char errBytes[16];
					int8_t errBytesLen = sizeof(errBytes);
					UErrorCode errBytesErr = U_ZERO_ERROR;

	 				ucnv_getInvalidChars(m_from, errBytes, &errBytesLen, &errBytesErr);

					st->inputBytesRead -= errBytesLen;
					st->outputBytesWritten += cpTarget - &cpOutBuffer[0];
				}

				// (*) If an error occurred while converting from input charset, throw it now
				if (toErr == U_INVALID_CHAR_FOUND ||
				    toErr == U_TRUNCATED_CHAR_FOUND ||
				    toErr == U_ILLEGAL_CHAR_FOUND) {

					throw exceptions::illegal_byte_sequence_for_charset();
				}

				if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr)) {

					if (fromErr == U_INVALID_CHAR_FOUND ||
					    fromErr == U_TRUNCATED_CHAR_FOUND ||
					    fromErr == U_ILLEGAL_CHAR_FOUND) {

						throw exceptions::illegal_byte_sequence_for_charset();

					} else {

						throw exceptions::charset_conv_error(
							"[ICU] Error converting from Unicode to " + m_dest.getName()
						);
					}
				}

				// Write to destination stream
				out.write(&cpOutBuffer[0], (cpTarget - &cpOutBuffer[0]));

			} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

		} while (toErr == U_BUFFER_OVERFLOW_ERROR);
	}
}


void charsetConverter_icu::convert(const string& in, string& out, status* st) {

	if (st) {
		new (st) status();
	}

	out.clear();

	utility::inputStreamStringAdapter is(in);
	utility::outputStreamStringAdapter os(out);

	convert(is, os, st);

	os.flush();
}


shared_ptr <utility::charsetFilteredOutputStream>
	charsetConverter_icu::getFilteredOutputStream(
		utility::outputStream& os,
		const charsetConverterOptions& opts
	) {

	return make_shared <utility::charsetFilteredOutputStream_icu>(m_source, m_dest, &os, opts);
}



// charsetFilteredOutputStream_icu

namespace utility {


charsetFilteredOutputStream_icu::charsetFilteredOutputStream_icu(
	const charset& source,
	const charset& dest,
	outputStream* os,
	const charsetConverterOptions& opts
)
	: m_from(NULL),
	  m_to(NULL),
	  m_sourceCharset(source),
	  m_destCharset(dest),
	  m_stream(*os),
	  m_options(opts) {

	UErrorCode err = U_ZERO_ERROR;
	m_from = ucnv_open(source.getName().c_str(), &err);

	if (!U_SUCCESS(err)) {

		throw exceptions::charset_conv_error(
			"Cannot initialize ICU converter for source charset '" + source.getName()
			+ "' (error code: " + u_errorName(err) + "."
		);
	}

	m_to = ucnv_open(dest.getName().c_str(), &err);

	if (!U_SUCCESS(err)) {

		throw exceptions::charset_conv_error(
			"Cannot initialize ICU converter for destination charset '" + dest.getName()
			+ "' (error code: " + u_errorName(err) + "."
		);
	}

	// Tell ICU what to do when encountering an illegal byte sequence
	if (m_options.silentlyReplaceInvalidSequences) {

		// Set replacement chars for when converting from Unicode to codepage
		icu::UnicodeString substString(m_options.invalidSequence.c_str());
		ucnv_setSubstString(m_to, substString.getTerminatedBuffer(), -1, &err);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting substitution string.");
		}

	} else {

		// Tell ICU top stop (and return an error) on illegal byte sequences
		ucnv_setToUCallBack(
			m_to, UCNV_TO_U_CALLBACK_STOP, UCNV_SUB_STOP_ON_ILLEGAL, NULL, NULL, &err
		);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting ToU callback.");
		}

		ucnv_setFromUCallBack(
			m_to, UCNV_FROM_U_CALLBACK_STOP, UCNV_SUB_STOP_ON_ILLEGAL, NULL, NULL, &err
		);

		if (U_FAILURE(err)) {
			throw exceptions::charset_conv_error("[ICU] Error when setting FromU callback.");
		}
	}
}


charsetFilteredOutputStream_icu::~charsetFilteredOutputStream_icu() {

	if (m_from) ucnv_close(m_from);
	if (m_to) ucnv_close(m_to);
}


outputStream& charsetFilteredOutputStream_icu::getNextOutputStream() {

	return m_stream;
}


void charsetFilteredOutputStream_icu::writeImpl(
	const byte_t* const data,
	const size_t count
) {

	if (!m_from || !m_to) {
		throw exceptions::charset_conv_error("Cannot initialize converters.");
	}

	// Allocate buffer for Unicode chars
	const size_t uniSize = ucnv_getMinCharSize(m_from) * count * sizeof(UChar);
	std::vector <UChar> uniBuffer(uniSize);

	// Conversion loop
	UErrorCode toErr = U_ZERO_ERROR;

	const char* uniSource = reinterpret_cast <const char*>(data);
	const char* uniSourceLimit = uniSource + count;

	do {

		// Convert from source charset to Unicode
		UChar* uniTarget = &uniBuffer[0];
		UChar* uniTargetLimit = &uniBuffer[0] + uniSize;

		toErr = U_ZERO_ERROR;

		ucnv_toUnicode(
			m_from, &uniTarget, uniTargetLimit,
			&uniSource, uniSourceLimit, NULL, /* flush */ UBool(0), &toErr
		);

		if (U_FAILURE(toErr) && toErr != U_BUFFER_OVERFLOW_ERROR) {

			if (toErr == U_INVALID_CHAR_FOUND ||
			    toErr == U_TRUNCATED_CHAR_FOUND ||
			    toErr == U_ILLEGAL_CHAR_FOUND) {

				throw exceptions::illegal_byte_sequence_for_charset();

			} else {

				throw exceptions::charset_conv_error(
					"[ICU] Error converting to Unicode from '" + m_sourceCharset.getName() + "'."
				);
			}
		}

		const size_t uniLength = uniTarget - &uniBuffer[0];
		if(uniLength == 0)
			continue;

		// Allocate buffer for destination charset
		const size_t cpSize = ucnv_getMinCharSize(m_to) * uniLength;
		std::vector <char> cpBuffer(cpSize);

		// Convert from Unicode to destination charset
		UErrorCode fromErr = U_ZERO_ERROR;

		const UChar* cpSource = &uniBuffer[0];
		const UChar* cpSourceLimit = &uniBuffer[0] + uniLength;

		do {

			char* cpTarget = &cpBuffer[0];
			char* cpTargetLimit = &cpBuffer[0] + cpSize;

			fromErr = U_ZERO_ERROR;

			ucnv_fromUnicode(
				m_to, &cpTarget, cpTargetLimit,
				&cpSource, cpSourceLimit, NULL, /* flush */ UBool(0), &fromErr
			);

			if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr)) {

				if (fromErr == U_INVALID_CHAR_FOUND ||
				    fromErr == U_TRUNCATED_CHAR_FOUND ||
				    fromErr == U_ILLEGAL_CHAR_FOUND) {

					throw exceptions::illegal_byte_sequence_for_charset();

				} else {

					throw exceptions::charset_conv_error(
						"[ICU] Error converting from Unicode to '" + m_destCharset.getName() + "'."
					);
				}
			}

			const size_t cpLength = cpTarget - &cpBuffer[0];

			// Write successfully converted bytes
			m_stream.write(&cpBuffer[0], cpLength);

		} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

	} while (toErr == U_BUFFER_OVERFLOW_ERROR);
}


void charsetFilteredOutputStream_icu::flush() {

	if (!m_from || !m_to) {
		throw exceptions::charset_conv_error("Cannot initialize converters.");
	}

	// Allocate buffer for Unicode chars
	const size_t uniSize = ucnv_getMinCharSize(m_from) * 1024 * sizeof(UChar);
	std::vector <UChar> uniBuffer(uniSize);

	// Conversion loop (with flushing)
	UErrorCode toErr = U_ZERO_ERROR;

	const char* uniSource = 0;
	const char* uniSourceLimit = 0;

	do {

		// Convert from source charset to Unicode
		UChar* uniTarget = &uniBuffer[0];
		UChar* uniTargetLimit = &uniBuffer[0] + uniSize;

		toErr = U_ZERO_ERROR;

		ucnv_toUnicode(
			m_from, &uniTarget, uniTargetLimit,
			&uniSource, uniSourceLimit, NULL, /* flush */ UBool(1), &toErr
		);

		if (U_FAILURE(toErr) && toErr != U_BUFFER_OVERFLOW_ERROR) {

			throw exceptions::charset_conv_error(
				"[ICU] Error converting to Unicode from '" + m_sourceCharset.getName() + "'."
			);
		}

		const size_t uniLength = uniTarget - &uniBuffer[0];
		if(uniLength == 0)
			continue;

		// Allocate buffer for destination charset
		const size_t cpSize = ucnv_getMinCharSize(m_to) * uniLength;
		std::vector <char> cpBuffer(cpSize);

		// Convert from Unicode to destination charset
		UErrorCode fromErr = U_ZERO_ERROR;

		const UChar* cpSource = &uniBuffer[0];
		const UChar* cpSourceLimit = &uniBuffer[0] + uniLength;

		do {

			char* cpTarget = &cpBuffer[0];
			char* cpTargetLimit = &cpBuffer[0] + cpSize;

			fromErr = U_ZERO_ERROR;

			ucnv_fromUnicode(
				m_to, &cpTarget, cpTargetLimit,
				&cpSource, cpSourceLimit, NULL, /* flush */ UBool(1), &fromErr
			);

			if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr)) {

				throw exceptions::charset_conv_error(
					"[ICU] Error converting from Unicode to '" + m_destCharset.getName() + "'."
				);
			}

			const size_t cpLength = cpTarget - &cpBuffer[0];

			// Write successfully converted bytes
			m_stream.write(&cpBuffer[0], cpLength);

		} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

	} while (toErr == U_BUFFER_OVERFLOW_ERROR);

	m_stream.flush();
}


} // utility


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_ICU
