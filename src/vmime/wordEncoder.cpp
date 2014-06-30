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

#include "vmime/wordEncoder.hpp"

#include "vmime/exception.hpp"
#include "vmime/charsetConverter.hpp"

#include "vmime/encoding.hpp"

#include "vmime/utility/encoder/b64Encoder.hpp"
#include "vmime/utility/encoder/qpEncoder.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime
{


wordEncoder::wordEncoder(const string& buffer, const charset& charset, const Encoding encoding)
	: m_buffer(buffer), m_pos(0), m_length(buffer.length()), m_charset(charset), m_encoding(encoding)
{
	try
	{
		string utf8Buffer;

		vmime::charset::convert
			(buffer, utf8Buffer, charset, vmime::charset(charsets::UTF_8));

		m_buffer = utf8Buffer;
		m_length = utf8Buffer.length();

		m_simple = false;
	}
	catch (exceptions::charset_conv_error&)
	{
		// Ignore exception.
		// We will fall back on simple encoding.
		m_simple = true;
	}

	if (m_encoding == ENCODING_AUTO)
		m_encoding = guessBestEncoding(buffer, charset);

	if (m_encoding == ENCODING_B64)
	{
		m_encoder = make_shared <utility::encoder::b64Encoder>();
	}
	else // ENCODING_QP
	{
		m_encoder = make_shared <utility::encoder::qpEncoder>();
		m_encoder->getProperties()["rfc2047"] = true;
	}
}


static size_t getUTF8CharLength
	(const string& buffer, const size_t pos, const size_t length)
{
	// Gives the number of extra bytes in a UTF8 char, given the leading char
	static const unsigned char UTF8_EXTRA_BYTES[256] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
	};

	const unsigned char c = buffer[pos];
	const unsigned char n = UTF8_EXTRA_BYTES[c];

	if (n < length - pos)
		return n + 1;
	else
		return 1;
}


const string wordEncoder::getNextChunk(const size_t maxLength)
{
	const size_t remaining = m_length - m_pos;

	if (remaining == 0)
		return string();

	vmime::string chunk;
	vmime::utility::outputStreamStringAdapter chunkStream(chunk);

	// Simple encoding
	if (m_simple)
	{
		// WARNING! Simple encoding can encode a non-integral number of
		// characters and then may generate incorrectly-formed words!

		if (m_encoding == ENCODING_B64)
		{
			// Here, we have a formula to compute the maximum number of source
			// bytes to encode knowing the maximum number of encoded chars. In
			// Base64 encoding, 3 bytes of input provide 4 bytes of output.
			const size_t inputCount =
				std::min(remaining, (maxLength > 1) ? ((maxLength - 1) * 3) / 4 : 1);

			// Encode chunk
			utility::inputStreamStringAdapter in(m_buffer, m_pos, m_pos + inputCount);

			m_encoder->encode(in, chunkStream);
			m_pos += inputCount;
		}
		else // ENCODING_QP
		{
			// Compute exactly how much input bytes are needed to have an output
			// string length of less than 'maxLength' bytes. In Quoted-Printable
			// encoding, encoded bytes take 3 bytes.
			size_t inputCount = 0;
			size_t outputCount = 0;

			while ((inputCount == 0 || outputCount < maxLength) && (inputCount < remaining))
			{
				const unsigned char c = m_buffer[m_pos + inputCount];

				inputCount++;
				outputCount += utility::encoder::qpEncoder::RFC2047_getEncodedLength(c);
			}

			// Encode chunk
			utility::inputStreamStringAdapter in(m_buffer, m_pos, m_pos + inputCount);

			m_encoder->encode(in, chunkStream);
			m_pos += inputCount;
		}
	}
	// Fully RFC-compliant encoding
	else
	{
		shared_ptr <charsetConverter> conv = charsetConverter::create(charsets::UTF_8, m_charset);

		size_t inputCount = 0;
		size_t outputCount = 0;
		string encodeBuffer;

		while ((inputCount == 0 || outputCount < maxLength) && (inputCount < remaining))
		{
			// Get the next UTF8 character
			const size_t inputCharLength =
				getUTF8CharLength(m_buffer, m_pos + inputCount, m_length);

			const string inputChar(m_buffer.begin() + m_pos + inputCount,
				m_buffer.begin() + m_pos + inputCount + inputCharLength);

			// Convert back to original encoding
			string encodeBytes;
			conv->convert(inputChar, encodeBytes);

			encodeBuffer += encodeBytes;

			// Compute number of output bytes
			if (m_encoding == ENCODING_B64)
			{
				outputCount = std::max(static_cast <size_t>(4),
					(encodeBuffer.length() * 4) / 3);
			}
			else // ENCODING_QP
			{
				for (size_t i = 0, n = encodeBytes.length() ; i < n ; ++i)
				{
					const unsigned char c = encodeBytes[i];
					outputCount += utility::encoder::qpEncoder::RFC2047_getEncodedLength(c);
				}
			}

			inputCount += inputCharLength;
		}

		// Encode chunk
		utility::inputStreamStringAdapter in(encodeBuffer);

		m_encoder->encode(in, chunkStream);
		m_pos += inputCount;
	}

	return chunk;
}


wordEncoder::Encoding wordEncoder::getEncoding() const
{
	return m_encoding;
}


// static
bool wordEncoder::isEncodingNeeded
	(const generationContext& ctx, const string& buffer,
	 const charset& charset, const string& lang)
{
	if (!ctx.getInternationalizedEmailSupport())
	{
		// Charset-specific encoding
		encoding recEncoding;

		if (charset.getRecommendedEncoding(recEncoding))
			return true;

		// No encoding is needed if the buffer only contains ASCII chars
		if (utility::stringUtils::findFirstNonASCIIchar(buffer.begin(), buffer.end()) != string::npos)
			return true;
	}

	// Force encoding when there are only ASCII chars, but there is
	// also at least one of '\n' or '\r' (header fields)
	if (buffer.find_first_of("\n\r") != string::npos)
		return true;

	// If any RFC-2047 sequence is found in the buffer, encode it
	if (buffer.find("=?") != string::npos || buffer.find("?=") != string::npos)
		return true;

	// If a language is specified, force encoding
	if (!lang.empty())
		return true;

	return false;
}


// static
wordEncoder::Encoding wordEncoder::guessBestEncoding
	(const string& buffer, const charset& charset)
{
	// Charset-specific encoding
	encoding recEncoding;

	if (charset.getRecommendedEncoding(recEncoding))
	{
		if (recEncoding == encoding(encodingTypes::QUOTED_PRINTABLE))
			return ENCODING_QP;
		else
			return ENCODING_B64;
	}

	// Use Base64 if more than 40% non-ASCII, or Quoted-Printable else (default)
	const size_t asciiCount =
		utility::stringUtils::countASCIIchars(buffer.begin(), buffer.end());

	const size_t asciiPercent =
		(buffer.length() == 0 ? 100 : (100 * asciiCount) / buffer.length());

	if (asciiPercent < 60)
		return ENCODING_B64;
	else
		return ENCODING_QP;
}


} // vmime

