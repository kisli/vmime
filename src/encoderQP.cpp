//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/encoderQP.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


encoderQP::encoderQP()
{
}


const std::vector <string> encoderQP::getAvailableProperties() const
{
	std::vector <string> list(encoder::getAvailableProperties());

	list.push_back("maxlinelength");

	list.push_back("text");  // if set, '\r' and '\n' are not hex-encoded.
	                         // WARNING! You should not use this for binary data!

	list.push_back("rfc2047");   // for header fields encoding (RFC #2047)

	return (list);
}



// Encoding table
const unsigned char encoderQP::sm_hexDigits[] = "0123456789ABCDEF";

// Decoding table
const unsigned char encoderQP::sm_hexDecodeTable[256] =
{
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
	 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};


#ifndef VMIME_BUILDING_DOC

#define QP_ENCODE_HEX(x) \
	outBuffer[outBufferPos] = '=';                           \
	outBuffer[outBufferPos + 1] = sm_hexDigits[x >> 4];  \
	outBuffer[outBufferPos + 2] = sm_hexDigits[x & 0xF]; \
	outBufferPos += 3;                                       \
	curCol += 3;

#define QP_WRITE(s, x, l) s.write(reinterpret_cast <utility::stream::value_type*>(x), l)

#endif // VMIME_BUILDING_DOC


const utility::stream::size_type encoderQP::encode(utility::inputStream& in, utility::outputStream& out)
{
	in.reset();  // may not work...

	const string::size_type propMaxLineLength =
		getProperties().getProperty <string::size_type>("maxlinelength", static_cast <string::size_type>(-1));

	const bool rfc2047 = getProperties().getProperty <bool>("rfc2047", false);
	const bool text = getProperties().getProperty <bool>("text", false);  // binary mode by default

	const bool cutLines = (propMaxLineLength != static_cast <string::size_type>(-1));
	const string::size_type maxLineLength = std::min(propMaxLineLength, static_cast <string::size_type>(74));

	// Process the data
	char buffer[16384];
	int bufferLength = 0;
	int bufferPos = 0;

	string::size_type curCol = 0;

	unsigned char outBuffer[16384];
	int outBufferPos = 0;

	utility::stream::size_type total = 0;

	while (bufferPos < bufferLength || !in.eof())
	{
		// Flush current output buffer
		if (outBufferPos + 6 >= static_cast <int>(sizeof(outBuffer)))
		{
			QP_WRITE(out, outBuffer, outBufferPos);

			total += outBufferPos;
			outBufferPos = 0;
		}

		// Need to get more data?
		if (bufferPos >= bufferLength)
		{
			bufferLength = in.read(buffer, sizeof(buffer));
			bufferPos = 0;

			// No more data
			if (bufferLength == 0)
				break;
		}

		// Get the next char and encode it
		const unsigned char c = static_cast <unsigned char>(buffer[bufferPos++]);

		switch (c)
		{
		case '.':
		{
			if (!rfc2047 && curCol == 0)
			{
				// If a '.' appears at the beginning of a line, we encode it to
				// to avoid problems with SMTP servers... ("\r\n.\r\n" means the
				// end of data transmission).
				QP_ENCODE_HEX('.')
				continue;
			}

			outBuffer[outBufferPos++] = '.';
			++curCol;
			break;
		}
		case ' ':
		{
			// RFC-2047, Page 5, 4.2. The "Q" encoding:
			// << The 8-bit hexadecimal value 20 (e.g., ISO-8859-1 SPACE) may be
			// represented as "_" (underscore, ASCII 95.). >>
			if (rfc2047)
			{
				outBuffer[outBufferPos++] = '_';
				++curCol;
			}
			else
			{
				// Need to get more data?
				if (bufferPos >= bufferLength)
				{
					bufferLength = in.read(buffer, sizeof(buffer));
					bufferPos = 0;
				}

				// Spaces cannot appear at the end of a line. So, encode the space.
				if (bufferPos >= bufferLength ||
				    (buffer[bufferPos] == '\r' || buffer[bufferPos] == '\n'))
				{
					QP_ENCODE_HEX(' ');
				}
				else
				{
					outBuffer[outBufferPos++] = ' ';
					++curCol;
				}
			}

			break;
		}
		case '\t':
		{
			QP_ENCODE_HEX(c)
			break;
		}
		case '\r':
		case '\n':
		{
			// Text mode (where using CRLF or LF or ... does not
			// care for a new line...)
			if (text)
			{
				outBuffer[outBufferPos++] = c;
				++curCol;
			}
			// Binary mode (where CR and LF bytes are important!)
			else
			{
				QP_ENCODE_HEX(c)
			}

			break;
		}
		case '=':
		{
			QP_ENCODE_HEX('=')
			break;
		}
		case ',':
		case ';':
		case ':':
		case '_':
		{
			if (rfc2047)
			{
				QP_ENCODE_HEX(c)
			}
			else
			{
				outBuffer[outBufferPos++] = c;
				++curCol;
			}

			break;
		}
		/*
			Rule #2: (Literal representation) Octets with decimal values of 33
			through 60 inclusive, and 62 through 126, inclusive, MAY be
			represented as the ASCII characters which correspond to those
			octets (EXCLAMATION POINT through LESS THAN, and GREATER THAN
			through TILDE, respectively).
		*/
		default:
		{
			//if ((c >= 33 && c <= 60) || (c >= 62 && c <= 126))
			if (c >= 33 && c <= 126 && c != 61)
			{
				outBuffer[outBufferPos++] = c;
				++curCol;
			}
			// Other characters: '=' + hexadecimal encoding
			else
			{
				QP_ENCODE_HEX(c)
			}

			break;
		}

		}

		// Soft line break : "=\r\n"
		if (cutLines && curCol >= maxLineLength - 1)
		{
			outBuffer[outBufferPos] = '=';
			outBuffer[outBufferPos + 1] = '\r';
			outBuffer[outBufferPos + 2] = '\n';

			outBufferPos += 3;
			curCol = 0;
		}
	}

	// Flush remaining output buffer
	if (outBufferPos != 0)
	{
		QP_WRITE(out, outBuffer, outBufferPos);
		total += outBufferPos;
	}

	return (total);
}


const utility::stream::size_type encoderQP::decode(utility::inputStream& in, utility::outputStream& out)
{
	in.reset();  // may not work...

	// Process the data
	const bool rfc2047 = getProperties().getProperty <bool>("rfc2047", false);

	char buffer[16384];
	int bufferLength = 0;
	int bufferPos = 0;

	unsigned char outBuffer[16384];
	int outBufferPos = 0;

	utility::stream::size_type total = 0;

	while (bufferPos < bufferLength || !in.eof())
	{
		// Flush current output buffer
		if (outBufferPos >= static_cast <int>(sizeof(outBuffer)))
		{
			QP_WRITE(out, outBuffer, outBufferPos);

			total += outBufferPos;
			outBufferPos = 0;
		}

		// Need to get more data?
		if (bufferPos >= bufferLength)
		{
			bufferLength = in.read(buffer, sizeof(buffer));
			bufferPos = 0;

			// No more data
			if (bufferLength == 0)
				break;
		}

		// Decode the next sequence (hex-encoded byte or printable character)
		unsigned char c = static_cast <unsigned char>(buffer[bufferPos++]);

		switch (c)
		{
		case '=':
		{
			if (bufferPos >= bufferLength)
			{
				bufferLength = in.read(buffer, sizeof(buffer));
				bufferPos = 0;
			}

			if (bufferPos < bufferLength)
			{
				c = static_cast <unsigned char>(buffer[bufferPos++]);

				switch (c)
				{
				// Ignore soft line break ("=\r\n" or "=\n")
				case '\r':

					// Read one byte more
					if (bufferPos >= bufferLength)
					{
						bufferLength = in.read(buffer, sizeof(buffer));
						bufferPos = 0;
					}

					if (bufferPos < bufferLength)
						++bufferPos;

					break;

				case '\n':

					break;

				// Hex-encoded char
				default:
				{
					// We need another byte...
					if (bufferPos >= bufferLength)
					{
						bufferLength = in.read(buffer, sizeof(buffer));
						bufferPos = 0;
					}

					if (bufferPos < bufferLength)
					{
						const unsigned char next = static_cast <unsigned char>(buffer[bufferPos++]);

						const unsigned char value =
							  sm_hexDecodeTable[c] * 16
							+ sm_hexDecodeTable[next];

						outBuffer[outBufferPos++] = value;
					}
					else
					{
						// Premature end-of-data
					}

					break;
				}

				}
			}
			else
			{
				// Premature end-of-data
			}

			break;
		}
		case '_':
		{
			if (rfc2047)
			{
				// RFC-2047, Page 5, 4.2. The "Q" encoding:
				// << Note that the "_" always represents hexadecimal 20, even if the SPACE
				// character occupies a different code position in the character set in use. >>
				outBuffer[outBufferPos++] = 0x20;
				break;
			}

			// no break here...
		}
		default:
		{
			outBuffer[outBufferPos++] = c;
		}

		}
	}

	// Flush remaining output buffer
	if (outBufferPos != 0)
	{
		QP_WRITE(out, outBuffer, outBufferPos);
		total += outBufferPos;
	}

	return (total);
}


} // vmime
