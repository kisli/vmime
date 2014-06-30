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

#include "vmime/word.hpp"
#include "vmime/text.hpp"

#include "vmime/utility/stringUtils.hpp"
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
	: m_charset(charset::getLocalCharset())
{
}


word::word(const word& w)
	: headerFieldValue(), m_buffer(w.m_buffer),
	  m_charset(w.m_charset), m_lang(w.m_lang)
{
}


word::word(const string& buffer) // Defaults to local charset
	: m_buffer(buffer), m_charset(charset::getLocalCharset())
{
}


word::word(const string& buffer, const charset& charset)
	: m_buffer(buffer), m_charset(charset)
{
}


word::word(const string& buffer, const charset& charset, const string& lang)
	: m_buffer(buffer), m_charset(charset), m_lang(lang)
{
}


shared_ptr <word> word::parseNext
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition,
	 bool prevIsEncoded, bool* isEncoded, bool isFirst)
{
	size_t pos = position;

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

	size_t startPos = pos;
	string unencoded;

	const charset defaultCharset = ctx.getInternationalizedEmailSupport()
		? charset(charsets::UTF_8) : charset(charsets::US_ASCII);

	while (pos < end)
	{
		// End of line: does not occur in the middle of an encoded word. This is
		// used to remove folding white-spaces from unencoded text.
		if (buffer[pos] == '\n')
		{
			size_t endPos = pos;

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
				if (prevIsEncoded && !isFirst)
					unencoded = whiteSpaces + unencoded;

				shared_ptr <word> w = make_shared <word>(unencoded, defaultCharset);
				w->setParsedBounds(position, pos);

				if (newPosition)
					*newPosition = pos;

				if (isEncoded)
					*isEncoded = false;

				return (w);
			}

			// ...else find the finish sequence '?=' and return an encoded word
			const size_t wordStart = pos;

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

			shared_ptr <word> w = make_shared <word>();
			w->parse(ctx, buffer, wordStart, pos, NULL);

			if (newPosition)
				*newPosition = pos;

			if (isEncoded)
				*isEncoded = true;

			return (w);
		}

		++pos;
	}

	if (startPos != end)
	{
		if (prevIsEncoded && !isFirst)
			unencoded = whiteSpaces + unencoded;

		unencoded += buffer.substr(startPos, end - startPos);
	}

	// Treat unencoded text at the end of the buffer
	if (!unencoded.empty())
	{
		shared_ptr <word> w = make_shared <word>(unencoded, defaultCharset);
		w->setParsedBounds(position, end);

		if (newPosition)
			*newPosition = end;

		if (isEncoded)
			*isEncoded = false;

		return (w);
	}

	return (null);
}


const std::vector <shared_ptr <word> > word::parseMultiple
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	std::vector <shared_ptr <word> > res;
	shared_ptr <word> w;

	size_t pos = position;

	bool prevIsEncoded = false;

	while ((w = word::parseNext(ctx, buffer, pos, end, &pos, prevIsEncoded, &prevIsEncoded, (w == NULL))) != NULL)
		res.push_back(w);

	if (newPosition)
		*newPosition = pos;

	return (res);
}


void word::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
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
						// Extract charset and language
						const string charsetAndLang(charsetPos, charsetEnd);
						const string::size_type asteriskPos = charsetAndLang.find('*');

						if (asteriskPos != string::npos)
						{
							m_charset = charset(string(charsetAndLang.begin(), charsetAndLang.begin() + asteriskPos));
							m_lang = string(charsetAndLang.begin() + asteriskPos + 1, charsetAndLang.end());
						}
						else
						{
							m_charset = charset(charsetAndLang);
							m_lang.clear();
						}

						// Decode text
						string decodedBuffer;

						utility::inputStreamStringAdapter ein(string(dataPos, dataEnd));
						utility::outputStreamStringAdapter eout(decodedBuffer);

						theEncoder->decode(ein, eout);
						delete (theEncoder);

						m_buffer = decodedBuffer;

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
	m_charset = ctx.getInternationalizedEmailSupport()
		? charset(charsets::UTF_8) : charset(charsets::US_ASCII);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void word::generateImpl(const generationContext& ctx, utility::outputStream& os,
	const size_t curLinePos, size_t* newLinePos) const
{
	generate(ctx, os, curLinePos, newLinePos, 0, NULL);
}


void word::generate(const generationContext& ctx, utility::outputStream& os,
	const size_t curLinePos, size_t* newLinePos, const int flags,
	generatorState* state) const
{
	size_t curLineLength = curLinePos;

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
		encodingNeeded = wordEncoder::isEncodingNeeded(ctx, m_buffer, m_charset, m_lang);

	// If text does not need to be encoded, quote the buffer (no folding is performed).
	if (!encodingNeeded &&
	    (flags & text::QUOTE_IF_NEEDED) &&
	    utility::stringUtils::needQuoting(m_buffer))
	{
		const string quoted = utility::stringUtils::quote(m_buffer, "\\\"", "\\");

		os << '"' << quoted << '"';
		curLineLength += 1 + quoted.length() + 1;
	}
	// If possible and requested (with flag), quote the buffer (no folding is performed).
	// Quoting is possible if and only if:
	//  - the buffer does not need to be encoded
	//  - the buffer does not contain quoting character (")
	//  - there is enough remaining space on the current line to hold the whole buffer
	else if (!encodingNeeded &&
	         (flags & text::QUOTE_IF_POSSIBLE) &&
	         m_buffer.find('"') == string::npos &&
	         (curLineLength + 2 /* 2 x " */ + m_buffer.length()) < ctx.getMaxLineLength())
	{
		os << '"' << m_buffer << '"';
		curLineLength += 2 + m_buffer.length();
	}
	// We will fold lines without encoding them.
	else if (!encodingNeeded)
	{
		string buffer;

		if (ctx.getInternationalizedEmailSupport())
		{
			// Convert the buffer to UTF-8
			charset::convert(m_buffer, buffer, m_charset, charsets::UTF_8);
		}
		else
		{
			// Leave the buffer as-is
			buffer = m_buffer;
		}

		// Here, we could have the following conditions:
		//
		//  * a maximum line length of N bytes
		//  * a buffer containing N+1 bytes, with no whitespace
		//
		// Look in the buffer for any run (ie. whitespace-separated sequence) which
		// is longer than the maximum line length. If there is one, then force encoding,
		// so that no generated line is longer than the maximum line length.
		size_t maxRunLength = 0;
		size_t curRunLength = 0;

		for (string::const_iterator p = buffer.begin(), end = buffer.end() ; p != end ; ++p)
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

		if (((flags & text::FORCE_NO_ENCODING) == 0) && maxRunLength >= ctx.getMaxLineLength() - 3)
		{
			// Generate with encoding forced
			generate(ctx, os, curLinePos, newLinePos, flags | text::FORCE_ENCODING, state);
			return;
		}

		// Output runs, and fold line when a whitespace is encountered
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
				if (curLineLength >= ctx.getMaxLineLength() && lastWSpos != end)
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
				    !state->isFirstWord && curLineStart == buffer.begin())
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

					if (p != buffer.begin() && parserHelpers::isSpace(*(p - 1)))
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

		const size_t maxLineLength3 =
			(ctx.getMaxLineLength() == lineLengthLimits::infinite)
				? ctx.getMaxLineLength()
				: std::min(ctx.getMaxLineLength(), static_cast <size_t>(76));

		wordEncoder wordEnc(m_buffer, m_charset);

		const string wordStart("=?"
			+ m_charset.getName()
			+ (m_lang.empty() ? "" : string("*") + m_lang)
			+ "?"
			+ (wordEnc.getEncoding() == wordEncoder::ENCODING_B64 ? 'B' : 'Q')
			+ "?");
		const string wordEnd("?=");

		const size_t minWordLength = wordStart.length() + wordEnd.length();
		const size_t maxLineLength2 = (maxLineLength3 < minWordLength + 1)
			? maxLineLength3 + minWordLength + 1 : maxLineLength3;

		// Checks whether remaining space on this line is usable. If too few
		// characters can be encoded, start a new line.
		bool startNewLine = true;

		if (curLineLength + 2 < maxLineLength2)
		{
			const size_t remainingSpaceOnLine = maxLineLength2 - curLineLength - 2;

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
			const size_t fit = maxLineLength2 - minWordLength
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
	m_lang = w.m_lang;
	return (*this);
}


word& word::operator=(const string& s)
{
	m_buffer = s;
	m_charset = charset::getLocalCharset();
	m_lang.clear();
	return (*this);
}


void word::copyFrom(const component& other)
{
	const word& w = dynamic_cast <const word&>(other);

	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
	m_lang = w.m_lang;
}


bool word::operator==(const word& w) const
{
	return (m_charset == w.m_charset && m_buffer == w.m_buffer && m_lang == w.m_lang);
}


bool word::operator!=(const word& w) const
{
	return (m_charset != w.m_charset || m_buffer != w.m_buffer || m_lang != w.m_lang);
}


bool word::isEquivalent(const word& other) const
{
	return getConvertedText(charset(charsets::UTF_8)) == other.getConvertedText(charset(charsets::UTF_8));
}


const string word::getConvertedText(const charset& dest, const charsetConverterOptions& opts) const
{
	if (dest == m_charset)
		return m_buffer;  // no conversion needed

	string out;

	try
	{
		charset::convert(m_buffer, out, m_charset, dest, opts);
	}
	catch (vmime::exceptions::charset_conv_error& e)
	{
		// Do not fail if charset is not recognized:
		// copy 'word' as raw text
		out = m_buffer;
	}

	return (out);
}


shared_ptr <component> word::clone() const
{
	return make_shared <word>(m_buffer, m_charset);
}


const charset& word::getCharset() const
{
	return (m_charset);
}


void word::setCharset(const charset& ch)
{
	m_charset = ch;
}


const string word::getLanguage() const
{
	return m_lang;
}


void word::setLanguage(const string& lang)
{
	m_lang = lang;
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


const std::vector <shared_ptr <component> > word::getChildComponents()
{
	return std::vector <shared_ptr <component> >();
}


} // vmime
