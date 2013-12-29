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


#if VMIME_CHARSETCONV_LIB_IS_ICONV


#include "vmime/charsetConverter_iconv.hpp"

#include "vmime/exception.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


extern "C"
{
#ifndef VMIME_BUILDING_DOC

	#include <iconv.h>
	#include <errno.h>

	// HACK: prototypes may differ depending on the compiler and/or system (the
	// second parameter may or may not be 'const'). This relies on the compiler
	// for choosing the right type.

	class ICONV_IN_TYPE
	{
	public:

		ICONV_IN_TYPE(const char** ptr) : m_ptr(ptr) { }

		ICONV_IN_TYPE(const vmime::byte_t** ptr)
			: m_ptr(reinterpret_cast <const char**>(ptr)) { }

		operator const char**() { return m_ptr; }
		operator char**() { return const_cast <char**>(m_ptr); }

	private:

		const char** m_ptr;
	};

	class ICONV_OUT_TYPE
	{
	public:

		ICONV_OUT_TYPE(char** ptr) : m_ptr(ptr) { }

		ICONV_OUT_TYPE(vmime::byte_t** ptr)
			: m_ptr(reinterpret_cast <char**>(ptr)) { }

		operator char**() { return m_ptr; }

	private:

		char** m_ptr;
	};

#endif // VMIME_BUILDING_DOC
}



// Output replacement char when an invalid sequence is encountered
template <typename OUTPUT_CLASS, typename ICONV_DESC>
void outputInvalidChar(OUTPUT_CLASS& out, ICONV_DESC cd,
                       const vmime::charsetConverterOptions& opts = vmime::charsetConverterOptions())
{
	const char* invalidCharIn = opts.invalidSequence.c_str();
	vmime::size_t invalidCharInLen = opts.invalidSequence.length();

	vmime::byte_t invalidCharOutBuffer[16];
	vmime::byte_t* invalidCharOutPtr = invalidCharOutBuffer;
	vmime::size_t invalidCharOutLen = 16;

	if (iconv(cd, ICONV_IN_TYPE(&invalidCharIn), &invalidCharInLen,
		ICONV_OUT_TYPE(&invalidCharOutPtr), &invalidCharOutLen) != static_cast <size_t>(-1))
	{
		out.write(invalidCharOutBuffer, 16 - invalidCharOutLen);
	}
}



namespace vmime
{


// static
shared_ptr <charsetConverter> charsetConverter::createGenericConverter
	(const charset& source, const charset& dest,
	 const charsetConverterOptions& opts)
{
	return make_shared <charsetConverter_iconv>(source, dest, opts);
}


charsetConverter_iconv::charsetConverter_iconv
	(const charset& source, const charset& dest, const charsetConverterOptions& opts)
	: m_desc(NULL), m_source(source), m_dest(dest), m_options(opts)
{
	// Get an iconv descriptor
	const iconv_t cd = iconv_open(dest.getName().c_str(), source.getName().c_str());

	if (cd != reinterpret_cast <iconv_t>(-1))
	{
		iconv_t* p = new iconv_t;
		*p= cd;

		m_desc = p;
	}
}


charsetConverter_iconv::~charsetConverter_iconv()
{
	if (m_desc != NULL)
	{
		// Close iconv handle
		iconv_close(*static_cast <iconv_t*>(m_desc));

		delete static_cast <iconv_t*>(m_desc);
		m_desc = NULL;
	}
}


void charsetConverter_iconv::convert(utility::inputStream& in, utility::outputStream& out)
{
	if (m_desc == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converter.");

	const iconv_t cd = *static_cast <iconv_t*>(m_desc);

	byte_t inBuffer[32768];
	byte_t outBuffer[32768];
	size_t inPos = 0;

	bool prevIsInvalid = false;
	bool breakAfterNext = false;

	while (true)
	{
		// Fullfill the buffer
		size_t inLength = static_cast <size_t>(in.read(inBuffer + inPos, sizeof(inBuffer) - inPos) + inPos);
		size_t outLength = sizeof(outBuffer);

		const byte_t* inPtr = breakAfterNext ? NULL : inBuffer;
		size_t *ptrLength = breakAfterNext ? NULL : &inLength;
		byte_t* outPtr = outBuffer;

		// Convert input bytes
		if (iconv(cd, ICONV_IN_TYPE(&inPtr), ptrLength,
			      ICONV_OUT_TYPE(&outPtr), &outLength) == static_cast <size_t>(-1))
		{
			// Illegal input sequence or input sequence has no equivalent
			// sequence in the destination charset.
			if (prevIsInvalid)
			{
				// Write successfully converted bytes
				out.write(outBuffer, sizeof(outBuffer) - outLength);

				// Output a special character to indicate we don't known how to
				// convert the sequence at this position
				outputInvalidChar(out, cd, m_options);

				// Skip a byte and leave unconverted bytes in the input buffer
				std::copy(const_cast <byte_t*>(inPtr + 1), inBuffer + sizeof(inBuffer), inBuffer);
				inPos = inLength - 1;
			}
			else
			{
				// Write successfully converted bytes
				out.write(outBuffer, sizeof(outBuffer) - outLength);

				// Leave unconverted bytes in the input buffer
				std::copy(const_cast <byte_t*>(inPtr), inBuffer + sizeof(inBuffer), inBuffer);
				inPos = inLength;

				if (errno != E2BIG)
					prevIsInvalid = true;
			}
		}
		else
		{
			// Write successfully converted bytes
			out.write(outBuffer, sizeof(outBuffer) - outLength);

			inPos = 0;
			prevIsInvalid = false;
		}

		if (breakAfterNext)
			break;

		// Check for end of data, loop again to flush stateful data from iconv
		if (in.eof() && inPos == 0)
			breakAfterNext = true;
	}
}


void charsetConverter_iconv::convert(const string& in, string& out)
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


shared_ptr <utility::charsetFilteredOutputStream> charsetConverter_iconv::getFilteredOutputStream(utility::outputStream& os)
{
	return make_shared <utility::charsetFilteredOutputStream_iconv>(m_source, m_dest, &os);
}



// charsetFilteredOutputStream_iconv

namespace utility {


charsetFilteredOutputStream_iconv::charsetFilteredOutputStream_iconv
	(const charset& source, const charset& dest, outputStream* os)
	: m_desc(NULL), m_sourceCharset(source), m_destCharset(dest),
	  m_stream(*os), m_unconvCount(0)
{
	// Get an iconv descriptor
	const iconv_t cd = iconv_open(dest.getName().c_str(), source.getName().c_str());

	if (cd != reinterpret_cast <iconv_t>(-1))
	{
		iconv_t* p = new iconv_t;
		*p= cd;

		m_desc = p;
	}
}


charsetFilteredOutputStream_iconv::~charsetFilteredOutputStream_iconv()
{
	if (m_desc != NULL)
	{
		// Close iconv handle
		iconv_close(*static_cast <iconv_t*>(m_desc));

		delete static_cast <iconv_t*>(m_desc);
		m_desc = NULL;
	}
}


outputStream& charsetFilteredOutputStream_iconv::getNextOutputStream()
{
	return m_stream;
}


void charsetFilteredOutputStream_iconv::writeImpl
	(const byte_t* const data, const size_t count)
{
	if (m_desc == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converter.");

	const iconv_t cd = *static_cast <iconv_t*>(m_desc);

	const byte_t* curData = data;
	size_t curDataLen = count;

	// If there is some unconverted bytes left, add more data from this
	// chunk to see if it can now be converted.
	while (m_unconvCount != 0 || curDataLen != 0)
	{
		if (m_unconvCount != 0)
		{
			// Check if an incomplete input sequence is larger than the
			// input buffer size: should not happen except if something
			// in the input sequence is invalid. If so, output a special
			// character and skip one byte in the invalid sequence.
			if (m_unconvCount >= sizeof(m_unconvBuffer))
			{
				outputInvalidChar(m_stream, cd);

				std::copy(m_unconvBuffer + 1,
					m_unconvBuffer + m_unconvCount, m_unconvBuffer);

				m_unconvCount--;
			}

			// Get more data
			const size_t remaining =
				std::min(curDataLen, sizeof(m_unconvBuffer) - m_unconvCount);

			std::copy(curData, curData + remaining, m_unconvBuffer + m_unconvCount);

			m_unconvCount += remaining;
			curDataLen -= remaining;
			curData += remaining;

			if (remaining == 0)
				return;  // no more data

			// Try a conversion
			const byte_t* inPtr = m_unconvBuffer;
			size_t inLength = m_unconvCount;
			byte_t* outPtr = m_outputBuffer;
			size_t outLength = sizeof(m_outputBuffer);

			const size_t inLength0 = inLength;

			if (iconv(cd, ICONV_IN_TYPE(&inPtr), &inLength,
			          ICONV_OUT_TYPE(&outPtr), &outLength) == static_cast <size_t>(-1))
			{
				const size_t inputConverted = inLength0 - inLength;

				// Write successfully converted bytes
				m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

				// Shift unconverted bytes
				std::copy(m_unconvBuffer + inputConverted,
					m_unconvBuffer + m_unconvCount, m_unconvBuffer);

				m_unconvCount -= inputConverted;

				continue;
			}

			// Write successfully converted bytes
			m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

			// Empty the unconverted buffer
			m_unconvCount = 0;
		}

		if (curDataLen == 0)
			return;  // no more data

		// Now, convert the current data buffer
		const byte_t* inPtr = curData;
		size_t inLength = std::min(curDataLen, sizeof(m_outputBuffer) / MAX_CHARACTER_WIDTH);
		byte_t* outPtr = m_outputBuffer;
		size_t outLength = sizeof(m_outputBuffer);

		const size_t inLength0 = inLength;

		if (iconv(cd, ICONV_IN_TYPE(&inPtr), &inLength,
		          ICONV_OUT_TYPE(&outPtr), &outLength) == static_cast <size_t>(-1))
		{
			// Write successfully converted bytes
			m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

			const size_t inputConverted = inLength0 - inLength;

			curData += inputConverted;
			curDataLen -= inputConverted;

			// Put one byte byte into the unconverted buffer so
			// that the next iteration fill it
			if (curDataLen != 0)
			{
				m_unconvCount = 1;
				m_unconvBuffer[0] = *curData;

				curData++;
				curDataLen--;
			}
		}
		else
		{
			// Write successfully converted bytes
			m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

			curData += inLength0;
			curDataLen -= inLength0;
		}
	}
}


void charsetFilteredOutputStream_iconv::flush()
{
	if (m_desc == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converter.");

	const iconv_t cd = *static_cast <iconv_t*>(m_desc);

	size_t offset = 0;

	// Process unconverted bytes
	while (m_unconvCount != 0)
	{
		// Try a conversion
		const byte_t* inPtr = m_unconvBuffer + offset;
		size_t inLength = m_unconvCount;
		byte_t* outPtr = m_outputBuffer;
		size_t outLength = sizeof(m_outputBuffer);

		const size_t inLength0 = inLength;

		if (iconv(cd, ICONV_IN_TYPE(&inPtr), &inLength, ICONV_OUT_TYPE(&outPtr), &outLength) == static_cast <size_t>(-1))
		{
			const size_t inputConverted = inLength0 - inLength;

			// Skip a "blocking" character
			if (inputConverted == 0)
			{
				outputInvalidChar(m_stream, cd);

				offset++;
				m_unconvCount--;
			}
			else
			{
				// Write successfully converted bytes
				m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

				offset += inputConverted;
				m_unconvCount -= inputConverted;
			}
		}
		else
		{
			// Write successfully converted bytes
			m_stream.write(m_outputBuffer, sizeof(m_outputBuffer) - outLength);

			m_unconvCount = 0;
		}
	}

	m_stream.flush();
}


} // utility


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_ICONV
