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

#include "config.hpp"

#include "charset.hpp"
#include "base.hpp"

#include "encoder.hpp"
#include "encoderB64.hpp"
#include "encoderQP.hpp"

#include "text.hpp"

#include "parserHelpers.hpp"

// For initializing
#include "encoderFactory.hpp"
#include "headerFieldFactory.hpp"
#include "parameterFactory.hpp"
#include "textPartFactory.hpp"
#include "options.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "messaging/serviceFactory.hpp"
#endif


namespace vmime
{


/** "Null" (empty) string.
  */
const string NULL_STRING;

#if VMIME_WIDE_CHAR_SUPPORT
	/** "Null" (empty) wide-char string.
	  */
	const wstring NULL_WSTRING;
#endif

/** "Null" (empty) text.
  */
const text NULL_TEXT;


/** Return the library name (eg. "libvmime").
  *
  * @return library name
  */
const string libname() { return (VMIME_PACKAGE); }

/** Return the library version (eg. "0.5.2").
  *
  * @return library version
  */
const string libversion() { return (VMIME_VERSION " (" __DATE__ " " __TIME__ ")"); }


// New line sequence to be used when folding header fields.
const string NEW_LINE_SEQUENCE("\r\n ");
const string::size_type NEW_LINE_SEQUENCE_LENGTH(1);   // space

/** The CR-LF sequence.
  */
const string CRLF("\r\n");


/** The current MIME version supported by VMime.
  */
const string MIME_VERSION("1.0");


// Line length limits
namespace lineLengthLimits
{
	const string::size_type infinite = std::numeric_limits <string::size_type>::max();
}



/** Test two strings for equality (case insensitive).
  * WARNING: use this with ASCII-only strings.
  *
  * @param s1 first string
  * @param s2 second string (must be in lower-case!)
  * @param n length of the second string
  * @return true if the two strings compare equally, false otherwise
  */

bool isStringEqualNoCase(const string& s1, const char* s2, const string::size_type n)
{
	// 'n' is the number of characters to compare
	// 's2' must be in lowercase letters only
	if (s1.length() < n)
		return (false);

	bool equal = true;

	for (string::size_type i = 0 ; equal && i < n ; ++i)
		equal = (std::tolower(s1[i], std::locale()) == s2[i]);

	return (equal);
}


/** Test two strings for equality (case insensitive).
  * WARNING: use this with ASCII-only strings.
  *
  * @param s1 first string
  * @param s2 second string
  * @return true if the two strings compare equally, false otherwise
  */

bool isStringEqualNoCase(const string& s1, const string& s2)
{
	if (s1.length() != s2.length())
		return (false);

	bool equal = true;
	const string::const_iterator end = s1.end();

	for (string::const_iterator i = s1.begin(), j = s2.begin(); i != end ; ++i, ++j)
		equal = (std::tolower(*i, std::locale()) == std::tolower(*j, std::locale()));

	return (equal);
}


/** Test two strings for equality (case insensitive).
  * WARNING: use this with ASCII-only strings.
  *
  * @param begin start position of the first string
  * @param end end position of the first string
  * @param s second string (must be in lower-case!)
  * @param n length of the second string
  * @return true if the two strings compare equally, false otherwise
  */

bool isStringEqualNoCase(const string::const_iterator begin, const string::const_iterator end,
	const char* s, const string::size_type n)
{
	if ((string::size_type)(end - begin) < n)
		return (false);

	bool equal = true;
	char* c = const_cast<char*>(s);
	string::size_type r = n;

	for (string::const_iterator i = begin ; equal && r && *c ; ++i, ++c, --r)
		equal = (std::tolower(*i, std::locale()) == *c);

	return (r == 0 && equal);
}


/** Transform all the characters in a string to lower-case.
  * WARNING: use this with ASCII-only strings.
  *
  * @param str the string to transform
  * @return a new string in lower-case
  */

const string toLower(const string& str)
{
	string out(str);
	const string::iterator end = out.end();

	for (string::iterator i = out.begin() ; i != end ; ++i)
		*i = std::tolower(*i, std::locale());

	return (out);
}


/** Strip the space characters (SPC, TAB, CR, LF) at the beginning
  * and at the end of the specified string.
  *
  * @param str string in which to strip spaces
  * @return a new string with space characters removed
  */

const string trim(const string& str)
{
	string::const_iterator b = str.begin();
	string::const_iterator e = str.end();

	if (b != e)
	{
		for ( ; b != e && isspace(*b) ; ++b);
		for ( ; e != b && isspace(*(e - 1)) ; --e);
	}

	return (string(b, e));
}


/** Return the number of 7-bit US-ASCII characters in a string.
  *
  * @param begin start position
  * @param end end position
  * @return number of ASCII characters
  */

string::size_type countASCIIchars
	(const string::const_iterator begin, const string::const_iterator end)
{
	string::size_type count = 0;

	for (string::const_iterator i = begin ; i != end ; ++i)
	{
		if (isascii(*i))
		{
			if (*i != '=' || *(i + 1) != '?') // To avoid bad behaviour...
				++count;
		}
	}

	return (count);
}


/** Encode and fold text in respect to RFC-2047.
  *
  * @param os output stream
  * @param in input text
  * @param maxLineLength maximum line length for output
  * @param firstLineOffset the first line length (may be useful if the current output line is not empty)
  * @param lastLineLength will receive the length of the last line written
  * @param flags encoding flags (see encodeAndFoldFlags)
  */

void encodeAndFoldText(utility::outputStream& os, const text& in, const string::size_type maxLineLength,
	const string::size_type firstLineOffset, string::size_type* lastLineLength, const int flags)
{
	string::size_type curLineLength = firstLineOffset;

	for (text::const_iterator wi = in.begin() ; wi != in.end() ; ++wi)
	{
		const word& w = *wi;
		const string& buffer = w.buffer();

		// Calculate the number of ASCII chars to check whether encoding is needed
		// and _which_ encoding to use.
		const string::size_type asciiCount = countASCIIchars(buffer.begin(), buffer.end());

		bool noEncoding = (flags & encodeAndFoldFlags::forceNoEncoding) ||
		    (!(flags & encodeAndFoldFlags::forceEncoding) && asciiCount == buffer.length());

		if (noEncoding)
		{
			// We will fold lines without encoding them.

			string::const_iterator lastWSpos = buffer.end(); // last white-space position
			string::const_iterator curLineStart = buffer.begin(); // current line start

			string::const_iterator p = buffer.begin();
			const string::const_iterator end = buffer.end();

			bool finished = false;
			bool newLine = false;

			while (!finished)
			{
				for ( ; p != end ; ++p, ++curLineLength)
				{
					// Exceeded maximum line length, but we have found a white-space
					// where we can cut the line...
					if (curLineLength >= maxLineLength && lastWSpos != end)
						break;

					if (*p == ' ' || *p == '\t')
					{
						// Remember the position of this white-space character
						lastWSpos = p;
					}
				}

				if (p != end)
					++curLineLength;

				//if (p == end || curLineLength >= maxLineLength)
				{
					if (p == end || lastWSpos == end)
					{
						// If we are here, it means that we have found no whitespace
						// before the first "maxLineLength" characters. In this case,
						// we write the full line no matter of the max line length...

						if (!newLine && p != end && lastWSpos == end &&
						    wi != in.begin() && curLineStart == buffer.begin())
						{
							// Here, we are continuing on the line of previous encoded
							// word, but there is not even enough space to put the
							// first word of this line, so we start a new line.
							if (flags & encodeAndFoldFlags::noNewLineSequence)
							{
								os << CRLF;
								curLineLength = 0;
							}
							else
							{
								os << NEW_LINE_SEQUENCE;
								curLineLength = NEW_LINE_SEQUENCE_LENGTH;
							}

							p = curLineStart;
							lastWSpos = end;
							newLine = true;
						}
						else
						{
							os << string(curLineStart, p);

							if (p == end)
							{
								finished = true;
							}
							else
							{
								if (flags & encodeAndFoldFlags::noNewLineSequence)
								{
									os << CRLF;
									curLineLength = 0;
								}
								else
								{
									os << NEW_LINE_SEQUENCE;
									curLineLength = NEW_LINE_SEQUENCE_LENGTH;
								}

								curLineStart = p;
								lastWSpos = end;
								newLine = true;
							}
						}
					}
					else
					{
						// In this case, there will not be enough space on the line for all the
						// characters _after_ the last white-space; so we cut the line at this
						// last white-space.

#if 1
						if (curLineLength != 1 && wi != in.begin())
							os << " "; // Separate from previous word
#endif

						os << string(curLineStart, lastWSpos);

						if (flags & encodeAndFoldFlags::noNewLineSequence)
						{
							os << CRLF;
							curLineLength = 0;
						}
						else
						{
							os << NEW_LINE_SEQUENCE;
							curLineLength = NEW_LINE_SEQUENCE_LENGTH;
						}

						curLineStart = lastWSpos + 1;

						p = lastWSpos + 1;
						lastWSpos = end;
						newLine = true;
					}
				}
			}
		}
		/*
			RFC #2047:
			4. Encodings

			Initially, the legal values for "encoding" are "Q" and "B".  These
			encodings are described below.  The "Q" encoding is recommended for
			use when most of the characters to be encoded are in the ASCII
			character set; otherwise, the "B" encoding should be used.
			Nevertheless, a mail reader which claims to recognize 'encoded-word's
			MUST be able to accept either encoding for any character set which it
			supports.
		*/
		else
		{
			// We will encode _AND_ fold lines

			/*
				RFC #2047:
				2. Syntax of encoded-words

				" While there is no limit to the length of a multiple-line header
				  field, each line of a header field that contains one or more
				  'encoded-word's is limited to 76 characters. "
			*/

			const string::size_type maxLineLength3 =
				(maxLineLength == lineLengthLimits::infinite)
					? maxLineLength
					: std::min(maxLineLength, (const string::size_type) 76);

			// Base64 if more than 60% non-ascii, quoted-printable else (default)
			const string::size_type asciiPercent = (100 * asciiCount) / buffer.length();
			const string::value_type encoding = (asciiPercent <= 40) ? 'B' : 'Q';

			string wordStart("=?" + w.charset().name() + "?" + encoding + "?");
			string wordEnd("?=");

			const string::size_type minWordLength = wordStart.length() + wordEnd.length();
			const string::size_type maxLineLength2 = (maxLineLength3 < minWordLength + 1)
				? maxLineLength3 + minWordLength + 1 : maxLineLength3;

			// Checks whether remaining space on this line is usable. If too few
			// characters can be encoded, start a new line.
			bool startNewLine = true;

			if (curLineLength + 2 < maxLineLength2)
			{
				const string::size_type remainingSpaceOnLine = maxLineLength2 - curLineLength - 2;

				if (remainingSpaceOnLine < minWordLength + 10)
				{
					// Space for no more than 10 encoded chars!
					// It is not worth while to continue on this line...
					startNewLine = true;
				}
				else
				{
					// OK, there is enough usable space on the current line.
					startNewLine = false;
				}
			}

			if (startNewLine)
			{
				os << NEW_LINE_SEQUENCE;
				curLineLength = NEW_LINE_SEQUENCE_LENGTH;
			}

			// Encode and fold input buffer
			string::const_iterator pos = buffer.begin();
			string::size_type remaining = buffer.length();

			encoder* theEncoder = ((encoding == 'B')
				? ((encoder*) new encoderB64)
				: ((encoder*) new encoderQP));

			string qpEncodedBuffer;

			if (encoding == 'Q')
			{
				theEncoder->properties()["rfc2047"] = true;

				// In the case of Quoted-Printable encoding, we cannot simply encode input
				// buffer line by line. So, we encode the whole buffer and we will fold it
				// in the next loop...
				utility::inputStreamStringAdapter in(buffer);
				utility::outputStreamStringAdapter out(qpEncodedBuffer);

				theEncoder->encode(in, out);

				pos = qpEncodedBuffer.begin();
				remaining = qpEncodedBuffer.length();
			}

#if 1
			if (curLineLength != 1 && wi != in.begin())
			{
				os << " "; // Separate from previous word
				++curLineLength;
			}
#endif

			for ( ; remaining ; )
			{
				// Start a new encoded word
				os << wordStart;
				curLineLength += minWordLength;

				// Compute the number of encoded chars that will fit on this line
				const string::size_type fit = maxLineLength2 - curLineLength;

				// Base-64 encoding
				if (encoding == 'B')
				{
					// TODO: WARNING! "Any encoded word which encodes a non-integral
					// number of characters or octets is incorrectly formed."

					// Here, we have a formula to compute the maximum number of source
					// characters to encode knowing the maximum number of encoded chars
					// (with Base64, 3 bytes of input provide 4 bytes of output).
					string::size_type count = (fit > 1) ? ((fit - 1) * 3) / 4 : 1;
					if (count > remaining) count = remaining;

					utility::inputStreamStringAdapter in
						(buffer, pos - buffer.begin(), pos - buffer.begin() + count);

					curLineLength += theEncoder->encode(in, os);

					pos += count;
					remaining -= count;
				}
				// Quoted-Printable encoding
				else
				{
					// TODO: WARNING! "Any encoded word which encodes a non-integral
					// number of characters or octets is incorrectly formed."

					// All we have to do here is to take a certain number of character
					// (that is less than or equal to "fit") from the QP encoded buffer,
					// but we also make sure not to fold a "=XY" encoded char.
					const string::const_iterator qpEnd = qpEncodedBuffer.end();
					string::const_iterator lastFoldPos = pos;
					string::const_iterator p = pos;
					string::size_type n = 0;

					while (n < fit && p != qpEnd)
					{
						if (*p == '=')
						{
							if (n + 3 >= fit)
							{
								lastFoldPos = p;
								break;
							}

							p += 3;
							n += 3;
						}
						else
						{
							++p;
							++n;
						}
					}

					if (lastFoldPos == pos)
						lastFoldPos = p;

					os << string(pos, lastFoldPos);

					curLineLength += (lastFoldPos - pos) + 1;

					pos += n;
					remaining -= n;
				}

				// End of the encoded word
				os << wordEnd;

				if (remaining)
				{
					os << NEW_LINE_SEQUENCE;
					curLineLength = NEW_LINE_SEQUENCE_LENGTH;
				}
			}

			delete (theEncoder);
  		}
	}

	if (lastLineLength)
		*lastLineLength = curLineLength;
}


void decodeAndUnfoldText(const string::const_iterator& inStart, const string::const_iterator& inEnd, text& out)
{
	// NOTE: See RFC-2047, Pages 11-12 for knowing about handling
	// of white-spaces between encoded words.

	out.clear();

	string::const_iterator p = inStart;
	const string::const_iterator end = inEnd;

	const charset defaultCharset(charsets::US_ASCII);
	charset prevWordCharset(defaultCharset);

	bool prevIsEncoded = false;

	string::const_iterator prevPos = p;

	for ( ; ; )
	{
		if (p == end || *p == '\n')
		{
			string::const_iterator textEnd = p;

			if (textEnd != inStart && *(textEnd - 1) == '\r')
				--textEnd;

			if (textEnd != prevPos)
			{
				if (out.size() && prevWordCharset == defaultCharset)
				{
					out.back().buffer() += string(prevPos, textEnd);
				}
				else
				{
					prevWordCharset = defaultCharset;
					out.append(word(string(prevPos, textEnd), defaultCharset));
					prevIsEncoded = false;
				}
			}

			if (p == end)
			{
				// Finished
				break;
			}

			// Skip the new-line character
			prevPos = ++p;
		}
		else if (*p == '=' && (p + 1) != end && *(p + 1) == '?')
		{
			string::const_iterator wordPos = p;
			p += 2; // skip '=?'

			if (p != end)
			{
				const string::const_iterator charsetPos = p;

				for ( ; p != end && *p != '?' ; ++p);

				if (p != end) // a charset is specified
				{
					const string::const_iterator charsetEnd = p;
					const string::const_iterator encPos = ++p; // skip '?'

					for ( ; p != end && *p != '?' ; ++p);

					if (p != end) // an encoding is specified
					{
						//const string::const_iterator encEnd = p;
						const string::const_iterator dataPos = ++p; // skip '?'

						for ( ; p != end && !(*p == '?' && *(p + 1) == '=') ; ++p);

						if (p != end) // some data is specified
						{
							const string::const_iterator dataEnd = p;
							p += 2; // skip '?='

							encoder* theEncoder = NULL;

							// Base-64 encoding
							if (*encPos == 'B' || *encPos == 'b')
							{
								theEncoder = new encoderB64;
							}
							// Quoted-Printable encoding
							else if (*encPos == 'Q' || *encPos == 'q')
							{
								theEncoder = new encoderQP;
								theEncoder->properties()["rfc2047"] = true;
							}

							if (theEncoder)
							{
								// Decode text
								string decodedBuffer;

								utility::inputStreamStringAdapter ein(string(dataPos, dataEnd));
								utility::outputStreamStringAdapter eout(decodedBuffer);

								theEncoder->decode(ein, eout);
								delete (theEncoder);

								// Append all the unencoded text before this word
								if (prevPos != wordPos)
								{
									string::const_iterator p = prevPos;

									if (prevIsEncoded)
									{
										// Check whether there are only white-spaces between
										// the two encoded words
										for ( ; (p != wordPos) && isspace(*p) ; ++p);
									}

									if (p != wordPos) // if not empty
									{
										if (out.size() && prevWordCharset == defaultCharset)
										{
											out.back().buffer() += string(prevPos, wordPos);
										}
										else
										{
											out.append(word(string(prevPos, wordPos), defaultCharset));
											prevWordCharset = defaultCharset;
										}
									}
								}

								// Append this fresh decoded word to output text
								charset thisCharset(string(charsetPos, charsetEnd));

								if (out.size() && prevWordCharset == thisCharset)
								{
									out.back().buffer() += decodedBuffer;
								}
								else
								{
									prevWordCharset = thisCharset;
									out.append(word(decodedBuffer, thisCharset));
								}

								// This word has been decoded: we can advance in the input buffer
								prevPos = p;
								prevIsEncoded = true;
							}
							else
							{
								// Unknown encoding: can't decode this word, we will
								// treat this word as ordinary text (RFC-2047, Page 9).
							}
						}
					}
				}
			}
		}
		else
		{
			++p;
		}

		for ( ; p != end && *p != '=' && *p != '\n' ; ++p);
	}
}


void decodeAndUnfoldText(const string& in, text& out)
{
	decodeAndUnfoldText(in.begin(), in.end(), out);
}


/** This function can be used to make several encoded words from a text.
  * All the characters in the text must be in the same specified charset.
  *
  * <p>Eg: giving:</p>
  * <pre>   &lt;iso-8859-1> "Linux dans un t'el'ephone mobile"
  *    ("=?iso-8859-1?Q?Linux_dans_un_t=E9l=E9phone_mobile?=")
  * </pre><p>it will return:</p>
  * <pre>   &lt:us-ascii>   "Linux dans un "
  *    &lt;iso-8859-1> "t'el'ephone "
  *    &lt;us-ascii>   "mobile"
  *    ("Linux dans un =?iso-8859-1?Q?t=E9l=E9phone_?= mobile")
  * </pre>
  *
  * @param in input string
  * @param ch input charset
  * @param out output text
  */

void makeWordsFromText(const string& in, const charset& ch, text& out)
{
	const string::const_iterator end = in.end();
	string::const_iterator p = in.begin();
	string::const_iterator start = in.begin();

	bool is8bit = false;     // is the current word 8-bit?
	bool prevIs8bit = false; // is previous word 8-bit?
	unsigned int count = 0;  // total number of words

	out.clear();

	for ( ; ; )
	{
		if (p == end || isspace(*p))
		{
			if (p != end)
				++p;

			if (is8bit)
			{
				if (prevIs8bit)
				{
					// No need to create a new encoded word, just append
					// the current word to the previous one.
					out.back().buffer() += string(start, p);
				}
				else
				{
					out.append(word(string(start, p), ch));
					prevIs8bit = true;
					++count;
				}
			}
			else
			{
				if (count && !prevIs8bit)
				{
					out.back().buffer() += string(start, p);
				}
				else
				{
					out.append(word(string(start, p), charset(charsets::US_ASCII)));
					prevIs8bit = false;
					++count;
				}
			}

			if (p == end)
				break;

			is8bit = false;
			start = p;
		}
		else if (!isascii(*p))
		{
			is8bit = true;
			++p;
		}
		else
		{
			++p;
		}
	}
}


//
//  V-Mime Initializer
// ====================
//
// Force instanciation of singletons. This is to prevent problems that might
// happen in multithreaded applications...
//
// WARNING: we put the initializer at the end of this compilation unit. This
// ensures this object is initialized _after_ all other global variables in
// the same compilation unit (in particular "lineLengthLimits::infinite",
// which is used by the generate() function (called from "textPartFactory"
// constructor, for example).
//

class initializer
{
public:

	initializer()
	{
		options::getInstance();

		encoderFactory::getInstance();
		headerFieldFactory::getInstance();
		parameterFactory::getInstance();
		textPartFactory::getInstance();

		#if VMIME_HAVE_MESSAGING_FEATURES
			messaging::serviceFactory::getInstance();
		#endif
	}
};

initializer theInitializer;


} // vmime
