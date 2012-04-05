//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/encoder/qpEncoder.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {
namespace encoder {


qpEncoder::qpEncoder()
{
}


const std::vector <string> qpEncoder::getAvailableProperties() const
{
	std::vector <string> list(encoder::getAvailableProperties());

	list.push_back("maxlinelength");

	list.push_back("text");  // if set, '\r' and '\n' are not hex-encoded.
	                         // WARNING! You should not use this for binary data!

	list.push_back("rfc2047");   // for header fields encoding (RFC #2047)

	return (list);
}



// Hex-encoding table
const unsigned char qpEncoder::sm_hexDigits[] = "0123456789ABCDEF";


// RFC-2047 encoding table: we always encode RFC-2047 using the restricted
// charset, that is the one used for 'phrase' in From/To/Cc/... headers.
//
// " The set of characters that may be used in a "Q"-encoded 'encoded-word'
//   is restricted to: <upper and lower case ASCII letters, decimal digits,
//   "!", "*", "+", "-", "/", "=", and "_" (underscore, ASCII 95.)>. "
//
// Two special cases:
// - encode space (32) as underscore (95)
// - encode underscore as hex (=5F)
//
// This is a quick lookup table:
//   '1' means "encode", '0' means "no encoding"
//
const unsigned char qpEncoder::sm_RFC2047EncodeTable[] =
{
	/*   0  NUL */ 1, /*   1  SOH */ 1, /*   2  STX */ 1, /*   3  ETX */ 1, /*   4  EOT */ 1, /*   5  ENQ */ 1,
	/*   6  ACK */ 1, /*   7  BEL */ 1, /*   8   BS */ 1, /*   9  TAB */ 1, /*  10   LF */ 1, /*  11   VT */ 1,
	/*  12   FF */ 1, /*  13   CR */ 1, /*  14   SO */ 1, /*  15   SI */ 1, /*  16  DLE */ 1, /*  17  DC1 */ 1,
	/*  18  DC2 */ 1, /*  19  DC3 */ 1, /*  20  DC4 */ 1, /*  21  NAK */ 1, /*  22  SYN */ 1, /*  23  ETB */ 1,
	/*  24  CAN */ 1, /*  25   EM */ 1, /*  26  SUB */ 1, /*  27  ESC */ 1, /*  28   FS */ 1, /*  29   GS */ 1,
	/*  30   RS */ 1, /*  31   US */ 1, /*  32 SPACE*/ 1, /*  33    ! */ 0, /*  34    " */ 1, /*  35    # */ 1,
	/*  36    $ */ 1, /*  37    % */ 1, /*  38    & */ 1, /*  39    ' */ 1, /*  40    ( */ 1, /*  41    ) */ 1,
	/*  42    * */ 0, /*  43    + */ 0, /*  44    , */ 1, /*  45    - */ 0, /*  46    . */ 1, /*  47    / */ 0,
	/*  48    0 */ 0, /*  49    1 */ 0, /*  50    2 */ 0, /*  51    3 */ 0, /*  52    4 */ 0, /*  53    5 */ 0,
	/*  54    6 */ 0, /*  55    7 */ 0, /*  56    8 */ 0, /*  57    9 */ 0, /*  58    : */ 1, /*  59    ; */ 1,
	/*  60    < */ 1, /*  61    = */ 1, /*  62    > */ 1, /*  63    ? */ 1, /*  64    @ */ 1, /*  65    A */ 0,
	/*  66    B */ 0, /*  67    C */ 0, /*  68    D */ 0, /*  69    E */ 0, /*  70    F */ 0, /*  71    G */ 0,
	/*  72    H */ 0, /*  73    I */ 0, /*  74    J */ 0, /*  75    K */ 0, /*  76    L */ 0, /*  77    M */ 0,
	/*  78    N */ 0, /*  79    O */ 0, /*  80    P */ 0, /*  81    Q */ 0, /*  82    R */ 0, /*  83    S */ 0,
	/*  84    T */ 0, /*  85    U */ 0, /*  86    V */ 0, /*  87    W */ 0, /*  88    X */ 0, /*  89    Y */ 0,
	/*  90    Z */ 0, /*  91    [ */ 1, /*  92    " */ 1, /*  93    ] */ 1, /*  94    ^ */ 1, /*  95    _ */ 1,
	/*  96    ` */ 1, /*  97    a */ 0, /*  98    b */ 0, /*  99    c */ 0, /* 100    d */ 0, /* 101    e */ 0,
	/* 102    f */ 0, /* 103    g */ 0, /* 104    h */ 0, /* 105    i */ 0, /* 106    j */ 0, /* 107    k */ 0,
	/* 108    l */ 0, /* 109    m */ 0, /* 110    n */ 0, /* 111    o */ 0, /* 112    p */ 0, /* 113    q */ 0,
	/* 114    r */ 0, /* 115    s */ 0, /* 116    t */ 0, /* 117    u */ 0, /* 118    v */ 0, /* 119    w */ 0,
	/* 120    x */ 0, /* 121    y */ 0, /* 122    z */ 0, /* 123    { */ 1, /* 124    | */ 1, /* 125    } */ 1,
	/* 126    ~ */ 1, /* 127  DEL */ 1
};


// Hex-decoding table
const unsigned char qpEncoder::sm_hexDecodeTable[256] =
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


// static
bool qpEncoder::RFC2047_isEncodingNeededForChar(const unsigned char c)
{
	return (c >= 128 || sm_RFC2047EncodeTable[c] != 0);
}


// static
int qpEncoder::RFC2047_getEncodedLength(const unsigned char c)
{
	if (c >= 128 || sm_RFC2047EncodeTable[c] != 0)
	{
		if (c == 32)  // space
		{
			// Encoded as "_"
			return 1;
		}
		else
		{
			// Hex encoding
			return 3;
		}
	}
	else
	{
		return 1;  // no encoding
	}
}


#ifndef VMIME_BUILDING_DOC

#define QP_ENCODE_HEX(x) \
	outBuffer[outBufferPos] = '=';                           \
	outBuffer[outBufferPos + 1] = sm_hexDigits[x >> 4];  \
	outBuffer[outBufferPos + 2] = sm_hexDigits[x & 0xF]; \
	outBufferPos += 3;                                       \
	curCol += 3

#define QP_WRITE(s, x, l) s.write(reinterpret_cast <utility::stream::value_type*>(x), l)

#endif // VMIME_BUILDING_DOC


utility::stream::size_type qpEncoder::encode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
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
	utility::stream::size_type inTotal = 0;

	if (progress)
		progress->start(0);

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

		if (rfc2047)
		{
			if (c >= 128 || sm_RFC2047EncodeTable[c] != 0)
			{
				if (c == 32)  // space
				{
					// RFC-2047, Page 5, 4.2. The "Q" encoding:
					// << The 8-bit hexadecimal value 20 (e.g., ISO-8859-1 SPACE) may be
					// represented as "_" (underscore, ASCII 95.). >>
					outBuffer[outBufferPos++] = '_';
					++curCol;
				}
				else
				{
					// Other characters: '=' + hexadecimal encoding
					QP_ENCODE_HEX(c);
				}
			}
			else
			{
				// No encoding
				outBuffer[outBufferPos++] = c;
				++curCol;
			}
		}
		else
		{
			switch (c)
			{
			case 46:  // .
			{
				if (curCol == 0)
				{
					// If a '.' appears at the beginning of a line, we encode it to
					// to avoid problems with SMTP servers... ("\r\n.\r\n" means the
					// end of data transmission).
					QP_ENCODE_HEX('.');
					continue;
				}

				outBuffer[outBufferPos++] = '.';
				++curCol;
				break;
			}
			case 32:  // space
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

				break;
			}
			case 9:   // TAB
			{
				QP_ENCODE_HEX(c);
				break;
			}
			case 13:  // CR
			case 10:  // LF
			{
				// RFC-2045/6.7(4)

				// Text data
				if (text && !rfc2047)
				{
					outBuffer[outBufferPos++] = c;
					++curCol;
				}
				// Binary data
				else
				{
					QP_ENCODE_HEX(c);
				}

				break;
			}
			case 61:  // =
			{
				QP_ENCODE_HEX('=');
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

				//if ((c >= 33 && c <= 60) || (c >= 62 && c <= 126))
				if (c >= 33 && c <= 126 && c != 61 && c != 63)
				{
					outBuffer[outBufferPos++] = c;
					++curCol;
				}
				// Other characters: '=' + hexadecimal encoding
				else
				{
					QP_ENCODE_HEX(c);
				}

				break;

			} // switch (c)

			// Soft line break : "=\r\n"
			if (cutLines && curCol >= maxLineLength - 1)
			{
				outBuffer[outBufferPos] = '=';
				outBuffer[outBufferPos + 1] = '\r';
				outBuffer[outBufferPos + 2] = '\n';

				outBufferPos += 3;
				curCol = 0;
			}

		} // !rfc2047

		++inTotal;

		if (progress)
			progress->progress(inTotal, inTotal);
	}

	// Flush remaining output buffer
	if (outBufferPos != 0)
	{
		QP_WRITE(out, outBuffer, outBufferPos);
		total += outBufferPos;
	}

	if (progress)
		progress->stop(inTotal);

	return (total);
}


utility::stream::size_type qpEncoder::decode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
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
	utility::stream::size_type inTotal = 0;

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

		++inTotal;

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

				++inTotal;

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
					{
						++bufferPos;
						++inTotal;
					}

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

						++inTotal;

						const unsigned char value = static_cast <unsigned char>
							(sm_hexDecodeTable[c] * 16 + sm_hexDecodeTable[next]);

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

		if (progress)
			progress->progress(inTotal, inTotal);
	}

	// Flush remaining output buffer
	if (outBufferPos != 0)
	{
		QP_WRITE(out, outBuffer, outBufferPos);
		total += outBufferPos;
	}

	if (progress)
		progress->stop(inTotal);

	return (total);
}


} // encoder
} // utility
} // vmime
