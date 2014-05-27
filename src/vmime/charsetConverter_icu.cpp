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

#include "vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_ICU


#include "vmime/charsetConverter_icu.hpp"

#include "vmime/exception.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


extern "C"
{
#ifndef VMIME_BUILDING_DOC

	#include <unicode/ucnv.h>
	#include <unicode/ucnv_err.h>

#endif // VMIME_BUILDING_DOC
}


#include <unicode/unistr.h>


namespace vmime
{


// static
shared_ptr <charsetConverter> charsetConverter::createGenericConverter
	(const charset& source, const charset& dest,
	 const charsetConverterOptions& opts)
{
	return make_shared <charsetConverter_icu>(source, dest, opts);
}


charsetConverter_icu::charsetConverter_icu
	(const charset& source, const charset& dest, const charsetConverterOptions& opts)
	: m_from(NULL), m_to(NULL), m_source(source), m_dest(dest), m_options(opts)
{
	UErrorCode err = U_ZERO_ERROR;
	m_from = ucnv_open(source.getName().c_str(), &err);

	if (!U_SUCCESS(err))
	{
		throw exceptions::charset_conv_error
			("Cannot initialize ICU converter for source charset '" + source.getName() + "' (error code: " + u_errorName(err) + ".");
	}

	m_to = ucnv_open(dest.getName().c_str(), &err);

	if (!U_SUCCESS(err))
	{
		throw exceptions::charset_conv_error
			("Cannot initialize ICU converter for destination charset '" + dest.getName() + "' (error code: " + u_errorName(err) + ".");
	}
}


charsetConverter_icu::~charsetConverter_icu()
{
	if (m_from) ucnv_close(m_from);
	if (m_to) ucnv_close(m_to);
}


void charsetConverter_icu::convert(utility::inputStream& in, utility::outputStream& out)
{
	UErrorCode err = U_ZERO_ERROR;

	// From buffers
	byte_t cpInBuffer[16]; // stream data put here
	const size_t outSize = ucnv_getMinCharSize(m_from) * sizeof(cpInBuffer) * sizeof(UChar);
	UChar uOutBuffer[outSize]; // Unicode chars end up here

	// To buffers
	// converted (char) data end up here
	const size_t cpOutBufferSz = ucnv_getMaxCharSize(m_to) * outSize;
	char cpOutBuffer[cpOutBufferSz];

	// Set replacement chars for when converting from Unicode to codepage
	icu::UnicodeString substString(m_options.invalidSequence.c_str());
	ucnv_setSubstString(m_to, substString.getTerminatedBuffer(), -1, &err);

	if (U_FAILURE(err))
		throw exceptions::charset_conv_error("[ICU] Error setting replacement char.");

	// Input data available
	while (!in.eof())
	{
		// Read input data into buffer
		size_t inLength = in.read(cpInBuffer, sizeof(cpInBuffer));

		// Beginning of read data
		const char* source = reinterpret_cast <const char*>(&cpInBuffer[0]);
		const char* sourceLimit = source + inLength; // end + 1

		UBool flush = in.eof();  // is this last run?

		UErrorCode toErr;

		// Loop until all source has been processed
		do
		{
			// Set up target pointers
			UChar* target = uOutBuffer;
			UChar* targetLimit = target + outSize;

			toErr = U_ZERO_ERROR;
			ucnv_toUnicode(m_from, &target, targetLimit,
			               &source, sourceLimit, NULL, flush, &toErr);

			if (toErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(toErr))
				throw exceptions::charset_conv_error("[ICU] Error converting to Unicode from " + m_source.getName());

			// The Unicode source is the buffer just written and the limit
			// is where the previous conversion stopped (target is moved in the conversion)
			const UChar* uSource = uOutBuffer;
			UChar* uSourceLimit = target;
			UErrorCode fromErr;

			// Loop until converted chars are fully written
			do
			{
				char* cpTarget = &cpOutBuffer[0];
				const char* cpTargetLimit = cpOutBuffer + cpOutBufferSz;

				fromErr = U_ZERO_ERROR;

				// Write converted bytes (Unicode) to destination codepage
				ucnv_fromUnicode(m_to, &cpTarget, cpTargetLimit,
				                 &uSource, uSourceLimit, NULL, flush, &fromErr);

				if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr))
					throw exceptions::charset_conv_error("[ICU] Error converting from Unicode to " + m_dest.getName());

				// Write to destination stream
				out.write(cpOutBuffer, (cpTarget - cpOutBuffer));

			} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

		} while (toErr == U_BUFFER_OVERFLOW_ERROR);
	}
}


void charsetConverter_icu::convert(const string& in, string& out)
{
	if (m_source == m_dest)
	{
		// No conversion needed
		out = in;
		return;
	}

	out.clear();

	utility::inputStreamStringAdapter is(in);
	utility::outputStreamStringAdapter os(out);

	convert(is, os);

	os.flush();
}


shared_ptr <utility::charsetFilteredOutputStream> charsetConverter_icu::getFilteredOutputStream(utility::outputStream& os)
{
	return make_shared <utility::charsetFilteredOutputStream_icu>(m_source, m_dest, &os);
}



// charsetFilteredOutputStream_icu

namespace utility {


charsetFilteredOutputStream_icu::charsetFilteredOutputStream_icu
	(const charset& source, const charset& dest, outputStream* os)
	: m_from(NULL), m_to(NULL), m_sourceCharset(source), m_destCharset(dest), m_stream(*os)
{
	UErrorCode err = U_ZERO_ERROR;
	m_from = ucnv_open(source.getName().c_str(), &err);

	if (!U_SUCCESS(err))
	{
		throw exceptions::charset_conv_error
			("Cannot initialize ICU converter for source charset '" + source.getName() + "' (error code: " + u_errorName(err) + ".");
	}

	m_to = ucnv_open(dest.getName().c_str(), &err);

	if (!U_SUCCESS(err))
	{
		throw exceptions::charset_conv_error
			("Cannot initialize ICU converter for destination charset '" + dest.getName() + "' (error code: " + u_errorName(err) + ".");
	}

	// Set replacement chars for when converting from Unicode to codepage
	icu::UnicodeString substString(vmime::charsetConverterOptions().invalidSequence.c_str());
	ucnv_setSubstString(m_to, substString.getTerminatedBuffer(), -1, &err);

	if (U_FAILURE(err))
		throw exceptions::charset_conv_error("[ICU] Error setting replacement char.");
}


charsetFilteredOutputStream_icu::~charsetFilteredOutputStream_icu()
{
	if (m_from) ucnv_close(m_from);
	if (m_to) ucnv_close(m_to);
}


outputStream& charsetFilteredOutputStream_icu::getNextOutputStream()
{
	return m_stream;
}


void charsetFilteredOutputStream_icu::writeImpl
	(const byte_t* const data, const size_t count)
{
	if (m_from == NULL || m_to == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converters.");

	// Allocate buffer for Unicode chars
	const size_t uniSize = ucnv_getMinCharSize(m_from) * count * sizeof(UChar);
	UChar uniBuffer[uniSize];

	// Conversion loop
	UErrorCode toErr = U_ZERO_ERROR;

	const char* uniSource = reinterpret_cast <const char*>(data);
	const char* uniSourceLimit = uniSource + count;

	do
	{
		// Convert from source charset to Unicode
		UChar* uniTarget = uniBuffer;
		UChar* uniTargetLimit = uniBuffer + uniSize;

		toErr = U_ZERO_ERROR;

		ucnv_toUnicode(m_from, &uniTarget, uniTargetLimit,
		               &uniSource, uniSourceLimit, NULL, /* flush */ FALSE, &toErr);

		if (U_FAILURE(toErr) && toErr != U_BUFFER_OVERFLOW_ERROR)
		{
			throw exceptions::charset_conv_error
				("[ICU] Error converting to Unicode from '" + m_sourceCharset.getName() + "'.");
		}

		const size_t uniLength = uniTarget - uniBuffer;

		// Allocate buffer for destination charset
		const size_t cpSize = ucnv_getMinCharSize(m_to) * uniLength;
		char cpBuffer[cpSize];

		// Convert from Unicode to destination charset
		UErrorCode fromErr = U_ZERO_ERROR;

		const UChar* cpSource = uniBuffer;
		const UChar* cpSourceLimit = uniBuffer + uniLength;

		do
		{
			char* cpTarget = cpBuffer;
			char* cpTargetLimit = cpBuffer + cpSize;

			fromErr = U_ZERO_ERROR;

			ucnv_fromUnicode(m_to, &cpTarget, cpTargetLimit,
							 &cpSource, cpSourceLimit, NULL, /* flush */ FALSE, &fromErr);

			if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr))
			{
				throw exceptions::charset_conv_error
					("[ICU] Error converting from Unicode to '" + m_destCharset.getName() + "'.");
			}

			const size_t cpLength = cpTarget - cpBuffer;

			// Write successfully converted bytes
			m_stream.write(cpBuffer, cpLength);

		} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

	} while (toErr == U_BUFFER_OVERFLOW_ERROR);
}


void charsetFilteredOutputStream_icu::flush()
{
	if (m_from == NULL || m_to == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converters.");

	// Allocate buffer for Unicode chars
	const size_t uniSize = ucnv_getMinCharSize(m_from) * 1024 * sizeof(UChar);
	UChar uniBuffer[uniSize];

	// Conversion loop (with flushing)
	UErrorCode toErr = U_ZERO_ERROR;

	const char* uniSource = 0;
	const char* uniSourceLimit = 0;

	do
	{
		// Convert from source charset to Unicode
		UChar* uniTarget = uniBuffer;
		UChar* uniTargetLimit = uniBuffer + uniSize;

		toErr = U_ZERO_ERROR;

		ucnv_toUnicode(m_from, &uniTarget, uniTargetLimit,
		               &uniSource, uniSourceLimit, NULL, /* flush */ TRUE, &toErr);

		if (U_FAILURE(toErr) && toErr != U_BUFFER_OVERFLOW_ERROR)
		{
			throw exceptions::charset_conv_error
				("[ICU] Error converting to Unicode from '" + m_sourceCharset.getName() + "'.");
		}

		const size_t uniLength = uniTarget - uniBuffer;

		// Allocate buffer for destination charset
		const size_t cpSize = ucnv_getMinCharSize(m_to) * uniLength;
		char cpBuffer[cpSize];

		// Convert from Unicode to destination charset
		UErrorCode fromErr = U_ZERO_ERROR;

		const UChar* cpSource = uniBuffer;
		const UChar* cpSourceLimit = uniBuffer + uniLength;

		do
		{
			char* cpTarget = cpBuffer;
			char* cpTargetLimit = cpBuffer + cpSize;

			fromErr = U_ZERO_ERROR;

			ucnv_fromUnicode(m_to, &cpTarget, cpTargetLimit,
							 &cpSource, cpSourceLimit, NULL, /* flush */ TRUE, &fromErr);

			if (fromErr != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(fromErr))
			{
				throw exceptions::charset_conv_error
					("[ICU] Error converting from Unicode to '" + m_destCharset.getName() + "'.");
			}

			const size_t cpLength = cpTarget - cpBuffer;

			// Write successfully converted bytes
			m_stream.write(cpBuffer, cpLength);

		} while (fromErr == U_BUFFER_OVERFLOW_ERROR);

	} while (toErr == U_BUFFER_OVERFLOW_ERROR);

	m_stream.flush();
}


} // utility


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_ICU
