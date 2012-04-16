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

#include "vmime/word.hpp"
#include "vmime/text.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/smartPtr.hpp"
#include "vmime/parserHelpers.hpp"

#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"

#include "vmime/utility/encoder/encoder.hpp"
#include "vmime/utility/encoder/b64Encoder.hpp"
#include "vmime/utility/encoder/qpEncoder.hpp"

#include "vmime/wordEncoder.hpp"


namespace vmime
{


word::word()
	: m_charset(charset::getLocaleCharset())
{
}


word::word(const word& w)
	: headerFieldValue(), m_buffer(w.m_buffer), m_charset(w.m_charset)
{
}


word::word(const string& buffer) // Defaults to locale charset
	: m_buffer(buffer), m_charset(charset::getLocaleCharset())
{
}


word::word(const string& buffer, const charset& charset)
	: m_buffer(buffer), m_charset(charset)
{
}


ref <word> word::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition,
	bool prevIsEncoded, bool* isEncoded, bool isFirst)
{
	string::size_type pos = position;

	// Ignore white-spaces:
	//   - before the first word
	//   - between two encoded words
	//   - after the last word
	string whiteSpaces;

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
	{
		whiteSpaces += buffer[pos];
		++pos;
	}

	string::size_type startPos = pos;
	string unencoded;

	while (pos < end)
	{
		// End of line: does not occur in the middle of an encoded word. This is
		// used to remove folding white-spaces from unencoded text.
		if (buffer[pos] == '\n')
		{
			string::size_type endPos = pos;

			if (pos > position && buffer[pos - 1] == '\r')
			{
				++pos;
				--endPos;
			}

			while (pos != end && parserHelpers::isSpace(buffer[pos]))
				++pos;

			unencoded += buffer.substr(startPos, endPos - startPos);

			if (pos != end)  // ignore white-spaces at end
				unencoded += ' ';

			startPos = pos;
			continue;
		}
		// Start of an encoded word
		else if (pos + 8 < end &&  // 8 = "=?(.+)?(.+)?(.*)?="
		         buffer[pos] == '=' && buffer[pos + 1] == '?')
		{
			// Check whether there is some unencoded text before
			unencoded += buffer.substr(startPos, pos - startPos);

			if (!unencoded.empty())
			{
				if (prevIsEncoded)
					unencoded = whiteSpaces + unencoded;

				ref <word> w = vmime::create <word>(unencoded, charset(charsets::US_ASCII));
				w->setParsedBounds(position, pos);

				if (newPosition)
					*newPosition = pos;

				if (isEncoded)
					*isEncoded = false;

				return (w);
			}

			// ...else find the finish sequence '?=' and return an encoded word
			const string::size_type wordStart = pos;

			pos += 2;

			while (pos < end && buffer[pos] != '?')
				++pos;

			if (pos < end)
			{
				++pos; // skip '?' between charset and encoding

				while (pos < end && buffer[pos] != '?')
					++pos;

				if (pos < end)
				{
					++pos; // skip '?' between encoding and encoded data
				}
			}

			while (pos < end)
			{
				if (buffer[pos] == '\n')
				{
					// End of line not allowed in the middle of an encoded word:
					// treat this text as unencoded text (see *).
					break;
				}
				else if (buffer[pos] == '?' && pos + 1 < end && buffer[pos + 1] == '=')
				{
					// Found the finish sequence
					break;
				}

				++pos;
			}

			if (pos == end) // not a valid word (no finish sequence)
				continue;
			else if (buffer[pos] == '\n')  // (*)
				continue;

			pos += 2; // ?=

			ref <word> w = vmime::create <word>();
			w->parse(buffer, wordStart, pos, NULL);

			if (newPosition)
				*newPosition = pos;

			if (isEncoded)
				*isEncoded = true;

			return (w);
		}

		++pos;
	}

	if (startPos != end && !isFirst && prevIsEncoded)
		unencoded += whiteSpaces;

	if (startPos != end)
		unencoded += buffer.substr(startPos, end - startPos);

	// Treat unencoded text at the end of the buffer
	if (!unencoded.empty())
	{
		ref <word> w = vmime::create <word>(unencoded, charset(charsets::US_ASCII));
		w->setParsedBounds(position, end);

		if (newPosition)
			*newPosition = end;

		if (isEncoded)
			*isEncoded = false;

		return (w);
	}

	return (null);
}


const std::vector <ref <word> > word::parseMultiple(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	std::vector <ref <word> > res;
	ref <word> w;

	string::size_type pos = position;

	bool prevIsEncoded = false;

	while ((w = word::parseNext(buffer, pos, end, &pos, prevIsEncoded, &prevIsEncoded, (w == NULL))) != NULL)
		res.push_back(w);

	if (newPosition)
		*newPosition = pos;

	return (res);
}


void word::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	if (position + 6 < end && // 6 = "=?(.+)?(.*)?="
	    buffer[position] == '=' && buffer[position + 1] == '?')
	{
		string::const_iterator p = buffer.begin() + position + 2;
		const string::const_iterator pend = buffer.begin() + end;

		const string::const_iterator charsetPos = p;

		for ( ; p != pend && *p != '?' ; ++p) {}

		if (p != pend) // a charset is specified
		{
			const string::const_iterator charsetEnd = p;
			const string::const_iterator encPos = ++p; // skip '?'

			for ( ; p != pend && *p != '?' ; ++p) {}

			if (p != pend) // an encoding is specified
			{
				//const string::const_iterator encEnd = p;
				const string::const_iterator dataPos = ++p; // skip '?'

				for ( ; p != pend && !(*p == '?' && *(p + 1) == '=') ; ++p) {}

				if (p != pend) // some data is specified
				{
					const string::const_iterator dataEnd = p;
					p += 2; // skip '?='

					utility::encoder::encoder* theEncoder = NULL;

					// Base-64 encoding
					if (*encPos == 'B' || *encPos == 'b')
					{
						theEncoder = new utility::encoder::b64Encoder();
					}
					// Quoted-Printable encoding
					else if (*encPos == 'Q' || *encPos == 'q')
					{
						theEncoder = new utility::encoder::qpEncoder();
						theEncoder->getProperties()["rfc2047"] = true;
					}

					if (theEncoder)
					{
						// Decode text
						string decodedBuffer;

						utility::inputStreamStringAdapter ein(string(dataPos, dataEnd));
						utility::outputStreamStringAdapter eout(decodedBuffer);

						theEncoder->decode(ein, eout);
						delete (theEncoder);

						m_buffer = decodedBuffer;
						m_charset = charset(string(charsetPos, charsetEnd));

						setParsedBounds(position, p - buffer.begin());

						if (newPosition)
							*newPosition = (p - buffer.begin());

						return;
					}
				}
			}
		}
	}

	// Unknown encoding or malformed encoded word: treat the buffer as ordinary text (RFC-2047, Page 9).
	m_buffer = string(buffer.begin() + position, buffer.begin() + end);
	m_charset = charsets::US_ASCII;

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void word::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	generate(os, maxLineLength, curLinePos, newLinePos, 0, NULL);
}


void word::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos, const int flags,
	generatorState* state) const
{
	string::size_type curLineLength = curLinePos;

	generatorState defaultGeneratorState;

	if (state == NULL)
		state = &defaultGeneratorState;

	// Find out if encoding is forced or required by contents + charset
	bool encodingNeeded = false;

	if ((flags & text::FORCE_NO_ENCODING) != 0)
		encodingNeeded = false;
	else if ((flags & text::FORCE_ENCODING) != 0)
		encodingNeeded = true;
	else  // auto-detect
		encodingNeeded = wordEncoder::isEncodingNeeded(m_buffer, m_charset);

	// If possible and requested (with flag), quote the buffer (no folding is performed).
	// Quoting is possible if and only if:
	//  - the buffer does not need to be encoded
	//  - the buffer does not contain quoting character (")
	//  - there is enough remaining space on the current line to hold the whole buffer
	if (!encodingNeeded &&
	    (flags & text::QUOTE_IF_POSSIBLE) &&
	    m_buffer.find('"') == string::npos &&
	    (curLineLength + 2 /* 2 x " */ + m_buffer.length()) < maxLineLength)
	{
		os << '"' << m_buffer << '"';
		curLineLength += 2 + m_buffer.length();
	}
	// We will fold lines without encoding them.
	else if (!encodingNeeded)
	{
		// Here, we could have the following conditions:
		//
		//  * a maximum line length of N bytes
		//  * a buffer containing N+1 bytes, with no whitespace
		//
		// Look in the buffer for any run (ie. whitespace-separated sequence) which
		// is longer than the maximum line length. If there is one, then force encoding,
		// so that no generated line is longer than the maximum line length.
		string::size_type maxRunLength = 0;
		string::size_type curRunLength = 0;

		for (string::const_iterator p = m_buffer.begin(), end = m_buffer.end() ; p != end ; ++p)
		{
			if (parserHelpers::isSpace(*p))
			{
				maxRunLength = std::max(maxRunLength, curRunLength);
				curRunLength = 0;
			}
			else
			{
				curRunLength++;
			}
		}

		maxRunLength = std::max(maxRunLength, curRunLength);

		if (((flags & text::FORCE_NO_ENCODING) == 0) && maxRunLength >= maxLineLength - 3)
		{
			// Generate with encoding forced
			generate(os, maxLineLength, curLinePos, newLinePos, flags | text::FORCE_ENCODING, state);
			return;
		}

		// Output runs, and fold line when a whitespace is encountered
		string::const_iterator lastWSpos = m_buffer.end(); // last white-space position
		string::const_iterator curLineStart = m_buffer.begin(); // current line start

		string::const_iterator p = m_buffer.begin();
		const string::const_iterator end = m_buffer.end();

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

			if (p == end || lastWSpos == end)
			{
				// If we are here, it means that we have found no whitespace
				// before the first "maxLineLength" characters. In this case,
				// we write the full line no matter of the max line length...

				if (!newLine && p != end && lastWSpos == end &&
				    !state->isFirstWord && curLineStart == m_buffer.begin())
				{
					// Here, we are continuing on the line of previous encoded
					// word, but there is not even enough space to put the
					// first word of this line, so we start a new line.
					if (flags & text::NO_NEW_LINE_SEQUENCE)
					{
						os << CRLF;
						curLineLength = 0;

						state->lastCharIsSpace = true;
					}
					else
					{
						os << NEW_LINE_SEQUENCE;
						curLineLength = NEW_LINE_SEQUENCE_LENGTH;

						state->lastCharIsSpace = true;
					}

					p = curLineStart;
					lastWSpos = end;
					newLine = true;
				}
				else
				{
					if (!state->isFirstWord && state->prevWordIsEncoded && !state->lastCharIsSpace && !parserHelpers::isSpace(*curLineStart))
						os << " "; // Separate from previous word

					os << string(curLineStart, p);

					if (p != m_buffer.begin() && parserHelpers::isSpace(*(p - 1)))
						state->lastCharIsSpace = true;
					else
						state->lastCharIsSpace = false;

					if (p == end)
					{
						finished = true;
					}
					else
					{
						if (flags & text::NO_NEW_LINE_SEQUENCE)
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
				if (curLineLength != NEW_LINE_SEQUENCE_LENGTH && !state->isFirstWord && state->prevWordIsEncoded)
					os << " "; // Separate from previous word
#endif

				os << string(curLineStart, lastWSpos);

				if (lastWSpos > curLineStart && parserHelpers::isSpace(*(lastWSpos - 1)))
					state->lastCharIsSpace = true;
				else
					state->lastCharIsSpace = false;

				if (flags & text::NO_NEW_LINE_SEQUENCE)
				{
					os << CRLF;
					curLineLength = 0;

					state->lastCharIsSpace = true;
				}
				else
				{
					os << NEW_LINE_SEQUENCE;
					curLineLength = NEW_LINE_SEQUENCE_LENGTH;

					state->lastCharIsSpace = true;
				}

				curLineStart = lastWSpos + 1;

				p = lastWSpos + 1;
				lastWSpos = end;
				newLine = true;
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
				: std::min(maxLineLength, static_cast <string::size_type>(76));

		wordEncoder wordEnc(m_buffer, m_charset);

		const string wordStart("=?" + m_charset.getName() + "?" +
			(wordEnc.getEncoding() == wordEncoder::ENCODING_B64 ? 'B' : 'Q') + "?");
		const string wordEnd("?=");

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

			state->lastCharIsSpace = true;
		}

		// Encode and fold input buffer
		if (!startNewLine && !state->isFirstWord && !state->lastCharIsSpace)
		{
			os << " "; // Separate from previous word
			++curLineLength;

			state->lastCharIsSpace = true;
		}

		for (unsigned int i = 0 ; ; ++i)
		{
			// Compute the number of encoded chars that will fit on this line
			const string::size_type fit = maxLineLength2 - minWordLength
				- (i == 0 ? curLineLength : NEW_LINE_SEQUENCE_LENGTH);

			// Get the next encoded chunk
			const string chunk = wordEnc.getNextChunk(fit);

			if (chunk.empty())
				break;

			// Start a new encoded word
			if (i != 0)
			{
				os << NEW_LINE_SEQUENCE;
				curLineLength = NEW_LINE_SEQUENCE_LENGTH;
			}

			os << wordStart;
			curLineLength += minWordLength;

			os << chunk;
			curLineLength += chunk.length();

			// End of the encoded word
			os << wordEnd;

			state->prevWordIsEncoded = true;
			state->lastCharIsSpace = false;
		}
	}

	if (newLinePos)
		*newLinePos = curLineLength;

	state->isFirstWord = false;
}


word& word::operator=(const word& w)
{
	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
	return (*this);
}


word& word::operator=(const string& s)
{
	m_buffer = s;
	return (*this);
}


void word::copyFrom(const component& other)
{
	const word& w = dynamic_cast <const word&>(other);

	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
}


bool word::operator==(const word& w) const
{
	return (m_charset == w.m_charset && m_buffer == w.m_buffer);
}


bool word::operator!=(const word& w) const
{
	return (m_charset != w.m_charset || m_buffer != w.m_buffer);
}


const string word::getConvertedText(const charset& dest) const
{
	string out;

	charset::convert(m_buffer, out, m_charset, dest);

	return (out);
}


ref <component> word::clone() const
{
	return vmime::create <word>(m_buffer, m_charset);
}


const charset& word::getCharset() const
{
	return (m_charset);
}


void word::setCharset(const charset& ch)
{
	m_charset = ch;
}


const string& word::getBuffer() const
{
	return (m_buffer);
}


string& word::getBuffer()
{
	return (m_buffer);
}


bool word::isEmpty() const
{
	return m_buffer.empty();
}


void word::setBuffer(const string& buffer)
{
	m_buffer = buffer;
}


const std::vector <ref <component> > word::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
