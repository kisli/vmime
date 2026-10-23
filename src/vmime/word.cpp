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


namespace vmime {


word::word()
	: m_charset(charset::getLocalCharset()) {

}


word::word(const word& w)
	: headerFieldValue(),
	  m_buffer(w.m_buffer),
	  m_charset(w.m_charset),
	  m_lang(w.m_lang) {

}


word::word(const string& buffer) // Defaults to local charset
	: m_buffer(buffer),
	  m_charset(charset::getLocalCharset()) {

}


word::word(const string& buffer, const charset& charset)
	: m_buffer(buffer),
	  m_charset(charset) {

}


word::word(const string& buffer, const charset& charset, const string& lang)
	: m_buffer(buffer),
	  m_charset(charset),
	  m_lang(lang) {

}


// static
charset word::unencodedCharset(const parsingContext& ctx, const string& buffer) {

	// Plain ASCII stays US-ASCII, so that it does not get encoded on output
	if (ctx.getInternationalizedEmailSupport() &&
	    utility::stringUtils::findFirstNonASCIIchar(buffer.begin(), buffer.end()) != string::npos) {

		return charset(charsets::UTF_8);
	}

	return charset(charsets::US_ASCII);
}


shared_ptr <word> word::parseNext(
	parsingContext& ctx,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition,
	parserState* state
) {

	size_t pos = position;

	// Ignore white-spaces:
	//   - before the first word
	//   - between two encoded words
	//   - after the last word
	// Always ignore newlines
	string whiteSpaces;

	while (pos < end && parserHelpers::isSpace(buffer[pos])) {

		if (buffer[pos] != '\r' && buffer[pos] != '\n') {  // do not include newlines
			whiteSpaces += buffer[pos];
		}

		++pos;
	}

	size_t startPos = pos;
	string unencoded;

	while (pos < end) {

		// End of line: does not occur in the middle of an encoded word. This is
		// used to remove folding white-spaces from unencoded text.
		if (buffer[pos] == '\n') {

			size_t endPos = pos;

			if (pos > position && buffer[pos - 1] == '\r') {
				++pos;
				--endPos;
			}

			while (pos != end && parserHelpers::isSpace(buffer[pos])) {
				++pos;
			}

			unencoded += buffer.substr(startPos, endPos - startPos);

			if (pos != end) {  // ignore white-spaces at end
				unencoded += ' ';
			}

			startPos = pos;
			continue;

		// Start of an encoded word
		} else if (pos + 8 < end &&  // 8 = "=?(.+)?(.+)?(.*)?="
		           buffer[pos] == '=' && buffer[pos + 1] == '?') {

			// Check whether there is some unencoded text before
			unencoded += buffer.substr(startPos, pos - startPos);

			if (!unencoded.empty()) {

				if (state->prevIsEncoded && !state->isFirst) {
					unencoded = whiteSpaces + unencoded;
				}

				shared_ptr <word> w = make_shared <word>(unencoded, unencodedCharset(ctx, unencoded));
				w->setParsedBounds(position, pos);

				if (newPosition) {
					*newPosition = pos;
				}

				state->prevIsEncoded = false;
				state->isFirst = false;

				return w;
			}

			// ...else find the finish sequence '?=' and return an encoded word
			const size_t wordStart = pos;

			pos += 2;

			while (pos < end && buffer[pos] != '?') {
				++pos;
			}

			if (pos < end) {

				++pos; // skip '?' between charset and encoding

				while (pos < end && buffer[pos] != '?') {
					++pos;
				}

				if (pos < end) {
					++pos; // skip '?' between encoding and encoded data
				}
			}

			while (pos < end) {

				if (buffer[pos] == '\n') {

					// End of line not allowed in the middle of an encoded word:
					// treat this text as unencoded text (see *).
					break;

				} else if (buffer[pos] == '?' && pos + 1 < end && buffer[pos + 1] == '=') {

					// Found the finish sequence
					break;
				}

				++pos;
			}

			if (pos == end) {  // not a valid word (no finish sequence)
				continue;
			} else if (buffer[pos] == '\n') {  // (*)
				continue;
			}

			pos += 2; // ?=

			shared_ptr <word> w = make_shared <word>();
			w->parseWithState(ctx, buffer, wordStart, pos, NULL, state);

			if (newPosition) {
				*newPosition = pos;
			}

			state->prevIsEncoded = true;
			state->isFirst = false;

			return w;
		}

		++pos;
	}

	if (startPos != end) {

		if (state->prevIsEncoded && !state->isFirst) {
			unencoded = whiteSpaces + unencoded;
		}

		unencoded += buffer.substr(startPos, end - startPos);
	}

	// Treat unencoded text at the end of the buffer
	if (!unencoded.empty()) {

		shared_ptr <word> w = make_shared <word>(unencoded, unencodedCharset(ctx, unencoded));
		w->setParsedBounds(position, end);

		if (newPosition) {
			*newPosition = end;
		}

		state->prevIsEncoded = false;
		state->isFirst = false;

		return w;
	}

	return null;
}


const std::vector <shared_ptr <word> > word::parseMultiple(
	parsingContext& ctx,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition
) {

	std::vector <shared_ptr <word> > res;
	shared_ptr <word> w;

	size_t pos = position;

	parserState state;

	while ((w = word::parseNext(ctx, buffer, pos, end, &pos, &state))) {
		res.push_back(w);
	}

	if (newPosition) {
		*newPosition = pos;
	}

	return res;
}


void word::parseImpl(
	parsingContext& ctx,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition
) {

	parseWithState(ctx, buffer, position, end, newPosition, NULL);
}


void word::parseWithState(
	parsingContext& ctx,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition,
	parserState* state
) {

	if (position + 6 < end && // 6 = "=?(.+)?(.*)?="
	    buffer[position] == '=' && buffer[position + 1] == '?') {

		string::const_iterator p = buffer.begin() + position + 2;
		const string::const_iterator pend = buffer.begin() + end;

		const string::const_iterator charsetPos = p;

		for ( ; p != pend && *p != '?' ; ++p) {}

		if (p != pend) {  // a charset is specified

			const string::const_iterator charsetEnd = p;
			const string::const_iterator encPos = ++p; // skip '?'

			for ( ; p != pend && *p != '?' ; ++p) {}

			if (p != pend) {  // an encoding is specified

				//const string::const_iterator encEnd = p;
				const string::const_iterator dataPos = ++p; // skip '?'

				for ( ; p != pend && !(*p == '?' && *(p + 1) == '=') ; ++p) {}

				if (p != pend) {  // some data is specified

					const string::const_iterator dataEnd = p;
					p += 2; // skip '?='

					scoped_ptr <utility::encoder::encoder> theEncoder;

					// Base-64 encoding
					if (*encPos == 'B' || *encPos == 'b') {

						theEncoder.reset(new utility::encoder::b64Encoder());

					// Quoted-Printable encoding
					} else if (*encPos == 'Q' || *encPos == 'q') {

						theEncoder.reset(new utility::encoder::qpEncoder());
						theEncoder->getProperties()["rfc2047"] = true;
					}

					if (theEncoder) {

						// Extract charset and language
						const string charsetAndLang(charsetPos, charsetEnd);
						const string::size_type asteriskPos = charsetAndLang.find('*');

						if (asteriskPos != string::npos) {

							m_charset = charset(string(charsetAndLang.begin(), charsetAndLang.begin() + asteriskPos));
							m_lang = string(charsetAndLang.begin() + asteriskPos + 1, charsetAndLang.end());

						} else {

							m_charset = charset(charsetAndLang);
							m_lang.clear();
						}

						// Decode text
						string encodedBuffer(dataPos, dataEnd);
						string decodedBuffer;

						if (state && !state->undecodedBytes.empty()) {

							encodedBuffer = state->undecodedBytes + encodedBuffer;
							state->undecodedBytes.clear();
						}

						utility::inputStreamStringAdapter ein(encodedBuffer);
						utility::outputStreamStringAdapter eout(decodedBuffer);

						const size_t decodedLen = theEncoder->decode(ein, eout);

						m_buffer = decodedBuffer;

						setParsedBounds(position, p - buffer.begin());

						if (newPosition) {
							*newPosition = (p - buffer.begin());
						}

						// For Base64 encoding, ensure all bytes have been decoded.
						// If there are remaining bytes, keep them for the next run.
						//
						// This allows decoding some insanities like:
						//     =?utf-8?B?5Lit5?= =?utf-8?B?paH?=
						if (*encPos == 'B' || *encPos == 'b') {

							const size_t actualEncodedLen = encodedBuffer.length();
							const size_t theoricalEncodedLen =
								((decodedLen + ((decodedLen % 3) ? (3 - (decodedLen % 3)) : 0) ) / 3) * 4;

							if (state && actualEncodedLen != theoricalEncodedLen) {
								state->undecodedBytes.assign(dataPos + theoricalEncodedLen, dataEnd);
							}
						}

						return;
					}
				}
			}
		}
	}

	// Unknown encoding or malformed encoded word: treat the buffer as ordinary text (RFC-2047, Page 9).
	m_buffer = string(buffer.begin() + position, buffer.begin() + end);
	m_charset = unencodedCharset(ctx, m_buffer);

	setParsedBounds(position, end);

	if (newPosition) {
		*newPosition = end;
	}
}


void word::generateImpl(
	const generationContext& ctx,
	utility::outputStream& os,
	const size_t curLinePos,
	size_t* newLinePos
) const {

	generate(ctx, os, curLinePos, newLinePos, 0, NULL);
}


enum QuoteResult {
	QUOTE_NOT_DONE,       // nothing written, fold the text unquoted
	QUOTE_DONE,
	QUOTE_NEEDS_ENCODING  // nothing written, a run does not fit on a line
};


static bool isWSP(const char c) {

	return c == ' ' || c == '\t';
}


// Whether "buffer" can only be written as a quoted-string: it contains
// specials (RFC 5322 §3.2.3), or white-space which would be read
// as a single space outside quotes (§3.2.2)
static bool phraseNeedsQuoting(const string& buffer) {

	if (utility::stringUtils::needQuoting(buffer, "()<>[]:;@\\,.\"\t")) {
		return true;
	}

	return !buffer.empty() &&
	       (buffer[0] == ' ' || buffer[buffer.length() - 1] == ' ' ||
	        buffer.find("  ") != string::npos);
}


enum FoldPoint {
	FOLD_NONE,
	FOLD_SPACE,     // before a single space
	FOLD_RUN_END,   // before the last space of a white-space run
	FOLD_TAB,       // before a tab, which a lenient parser reads as a space
	FOLD_BACKSLASH  // after '\', which a lenient parser takes as continuation
};


// Where a fold may be put before buffer[pos], "prev" being the character
// before buffer[0]. Only at the end of a run, which lenient parsers
// collapse, and the kind tells how much a fold there is preferred.
static FoldPoint getFoldPoint(const string& buffer, const size_t pos, const char prev) {

	const char before = (pos == 0 ? prev : buffer[pos - 1]);

	if (!isWSP(buffer[pos]) ||
	    (pos + 1 < buffer.length() && isWSP(buffer[pos + 1])) ||
	    (pos == 0 && isWSP(before))) {

		return FOLD_NONE;
	}

	if (before == '\\') {
		return FOLD_BACKSLASH;
	} else if (buffer[pos] == '\t') {
		return FOLD_TAB;
	}

	return isWSP(before) ? FOLD_RUN_END : FOLD_SPACE;
}


// Position after "pos" where a quoted-string may be folded, or the end
static size_t nextFoldPoint(const string& quoted, size_t pos) {

	for (++pos ; pos < quoted.length() ; ++pos) {

		if (getFoldPoint(quoted, pos, 'x') == FOLD_SPACE) {
			break;
		}
	}

	return std::min(pos, quoted.length());
}


// Write "buffer" as a quoted-string (RFC 5322 §3.2.4). If it does
// not fit on the current line, fold before it, or inside it before
// white-space, where unfolding restores the text exactly.
static QuoteResult generateQuotedPhrase(
	const string& buffer,
	const generationContext& ctx,
	utility::outputStream& os,
	size_t& curLineLength,
	const int flags,
	word::generatorState* state
) {

	const size_t maxLineLength = ctx.getMaxLineLength();
	const string quoted = utility::stringUtils::quote(buffer, "\\\"", "\\");

	if (curLineLength + 2 /* 2 x " */ + quoted.length() > maxLineLength) {

		if (!phraseNeedsQuoting(buffer)) {
			return QUOTE_NOT_DONE;
		}

		// Lengths of the first and the longest run, with the quotes
		size_t firstRun = 0, maxRun = 0;

		for (size_t i = 0, next ; i < quoted.length() ; i = next) {

			next = nextFoldPoint(quoted, i);

			const size_t len = next - i + (i == 0 ? 1 : 0) + (next == quoted.length() ? 1 : 0);

			if (i == 0) {
				firstRun = len;
			}

			maxRun = std::max(maxRun, len);
		}

		// Same rule as for unquoted text
		if ((flags & text::FORCE_NO_ENCODING) == 0 && maxRun >= maxLineLength - 3) {
			return QUOTE_NEEDS_ENCODING;
		}

		// Start a new line if not even the first run fits on this one, or if
		// the quoted string then fits entirely, but leave a fold before the
		// first word to the enclosing component, and prefer not to fold
		// after a '\'
		if (curLineLength > NEW_LINE_SEQUENCE_LENGTH &&
		    (curLineLength + firstRun > maxLineLength ||
		     (!state->isFirstWord && !state->lastCharIsBackslash &&
		      NEW_LINE_SEQUENCE_LENGTH + 2 + quoted.length() <= maxLineLength))) {

			os << NEW_LINE_SEQUENCE;
			curLineLength = NEW_LINE_SEQUENCE_LENGTH;
		}
	}

	os << '"';
	++curLineLength;

	for (size_t i = 0, next ; i < quoted.length() ; i = next) {

		next = nextFoldPoint(quoted, i);

		const size_t len = next - i + (next == quoted.length() ? 1 : 0);

		if (i != 0 && curLineLength + len > maxLineLength) {
			os << CRLF;
			curLineLength = 0;
		}

		os << string(quoted, i, next - i);
		curLineLength += next - i;
	}

	os << '"';
	++curLineLength;

	state->prevWordIsEncoded = false;
	state->lastCharIsSpace = false;
	state->lastCharIsBackslash = false;

	return QUOTE_DONE;
}


void word::generate(
	const generationContext& ctx,
	utility::outputStream& os,
	const size_t curLinePos,
	size_t* newLinePos,
	const int flags,
	generatorState* state
) const {

	size_t curLineLength = curLinePos;

	generatorState defaultGeneratorState;

	if (!state) {
		state = &defaultGeneratorState;
	}

	// Nothing to write, keep the state of the previous word
	if (m_buffer.empty() && (flags & (text::QUOTE_IF_POSSIBLE | text::QUOTE_IF_NEEDED)) == 0) {

		if (newLinePos) {
			*newLinePos = curLinePos;
		}

		return;
	}

	// Find out if encoding is forced or required by contents + charset
	bool encodingNeeded = false;

	if ((flags & text::FORCE_NO_ENCODING) != 0) {
		encodingNeeded = false;
	} else if ((flags & text::FORCE_ENCODING) != 0) {
		encodingNeeded = true;
	} else {  // auto-detect
		encodingNeeded = wordEncoder::isEncodingNeeded(ctx, m_buffer, m_charset, m_lang);
	}

	QuoteResult quoteResult = QUOTE_NOT_DONE;

	// If text does not need to be encoded, quote the buffer (no folding is performed).
	if (!encodingNeeded &&
	    (flags & text::QUOTE_IF_NEEDED) &&
	    utility::stringUtils::needQuoting(m_buffer)) {

		const string quoted = utility::stringUtils::quote(m_buffer, "\\\"", "\\");

		os << '"' << quoted << '"';
		curLineLength += 1 + quoted.length() + 1;

		state->prevWordIsEncoded = false;
		state->lastCharIsBackslash = false;

	// If possible and requested (with flag), quote the buffer.
	// Quoting is possible if and only if:
	//  - the buffer does not need to be encoded
	//  - it fits on the current line, or it cannot be written unquoted
	} else if (!encodingNeeded &&
	           (flags & text::QUOTE_IF_POSSIBLE) &&
	           (quoteResult = generateQuotedPhrase(m_buffer, ctx, os, curLineLength,
	                                               flags, state)) != QUOTE_NOT_DONE) {

		if (quoteResult == QUOTE_NEEDS_ENCODING) {
			generate(ctx, os, curLinePos, newLinePos, flags | text::FORCE_ENCODING, state);
			return;
		}

	// We will fold lines without encoding them.
	} else if (!encodingNeeded) {

		string buffer;

		if (ctx.getInternationalizedEmailSupport()) {

			// Convert the buffer to UTF-8
			charset::convert(m_buffer, buffer, m_charset, charsets::UTF_8);

		} else {

			// Leave the buffer as-is
			buffer = m_buffer;
		}

		// Here, we could have the following conditions:
		//
		//  * a maximum line length of N bytes
		//  * a buffer containing N+1 bytes, with no whitespace
		//
		// Look in the buffer for any run (ie. sequence between two fold points) which
		// is longer than the maximum line length. If there is one, then force encoding,
		// so that no generated line is longer than the maximum line length.
		const char prevChar = state->lastCharIsBackslash ? '\\'
			: (state->lastCharIsSpace ? ' ' : 'x');

		size_t maxRunLength = 0;
		size_t curRunLength = 0;

		for (size_t i = 0 ; i < buffer.length() ; ++i) {

			if (getFoldPoint(buffer, i, prevChar) != FOLD_NONE) {

				maxRunLength = std::max(maxRunLength, curRunLength);
				curRunLength = 0;

			} else {

				curRunLength++;
			}
		}

		maxRunLength = std::max(maxRunLength, curRunLength);

		if (((flags & text::FORCE_NO_ENCODING) == 0) && maxRunLength >= ctx.getMaxLineLength() - 3) {

			// Generate with encoding forced
			generate(ctx, os, curLinePos, newLinePos, flags | text::FORCE_ENCODING, state);
			return;
		}

		// Separate from previous word, unless the text starts with white-space
		bool sepPending =
			!state->isFirstWord &&
			(state->prevWordIsEncoded || ctx.getInternationalizedEmailSupport()) &&
			!state->lastCharIsSpace &&
			!buffer.empty() && !parserHelpers::isSpace(buffer[0]);

		if (sepPending) {
			++curLineLength;
		}

		// Output runs, and fold line when a whitespace is encountered
		string::const_iterator lastWSpos = buffer.end(); // last white-space position
		string::const_iterator lastRunEnd = buffer.end(), lastTab = buffer.end();
		string::const_iterator lastBackslash = buffer.end();
		string::const_iterator curLineStart = buffer.begin(); // current line start

		string::const_iterator p = buffer.begin();
		const string::const_iterator end = buffer.end();

		bool finished = false;
		bool newLine = false;

		while (!finished) {

			for ( ; p != end ; ++p, ++curLineLength) {

				// Exceeded maximum line length, but we have found a white-space
				// where we can cut the line: a single space, else the end of a
				// run, else a tab, else after a '\'
				if (curLineLength >= ctx.getMaxLineLength()) {

					if (lastWSpos == end) {
						lastWSpos = lastRunEnd;
					}

					if (lastWSpos == end) {
						lastWSpos = lastTab;
					}

					if (lastWSpos == end) {
						lastWSpos = lastBackslash;
					}

					if (lastWSpos != end) {
						break;
					}
				}

				if (newLine && p == curLineStart) {
					continue;  // a fold was just put here
				}

				switch (getFoldPoint(buffer, p - buffer.begin(), prevChar)) {

					case FOLD_SPACE: lastWSpos = p; break;
					case FOLD_RUN_END: lastRunEnd = p; break;
					case FOLD_TAB: lastTab = p; break;
					case FOLD_BACKSLASH: lastBackslash = p; break;
					case FOLD_NONE: break;
				}
			}

			if (p != end) {
				++curLineLength;
			}

			if (p == end || lastWSpos == end) {

				// If we are here, it means that we have found no whitespace
				// before the first "maxLineLength" characters. In this case,
				// we write the full line no matter of the max line length...

				if (!newLine && p != end && lastWSpos == end &&
				    !state->isFirstWord && curLineStart == buffer.begin()) {

					// Here, we are continuing on the line of previous encoded
					// word, but there is not even enough space to put the
					// first word of this line, so we start a new line.
					if (flags & text::NO_NEW_LINE_SEQUENCE) {

						os << CRLF;
						curLineLength = 0;

						state->lastCharIsSpace = true;

					} else {

						os << NEW_LINE_SEQUENCE;
						curLineLength = NEW_LINE_SEQUENCE_LENGTH;

						state->lastCharIsSpace = true;
					}

					p = curLineStart;
					lastWSpos = lastRunEnd = lastTab = lastBackslash = end;
					newLine = true;
					sepPending = false;

				} else {

					if (sepPending) {
						os << " "; // Separate from previous word
						sepPending = false;
					}

					os << string(curLineStart, p);

					if (p != buffer.begin() && parserHelpers::isSpace(*(p - 1))) {
						state->lastCharIsSpace = true;
					} else {
						state->lastCharIsSpace = false;
					}

					if (p == end) {

						finished = true;

					} else {

						if (flags & text::NO_NEW_LINE_SEQUENCE) {

							os << CRLF;
							curLineLength = 0;

						} else {

							os << NEW_LINE_SEQUENCE;
							curLineLength = NEW_LINE_SEQUENCE_LENGTH;
						}

						curLineStart = p;
						lastWSpos = lastRunEnd = lastTab = lastBackslash = end;
						newLine = true;
					}
				}

			} else {

				// In this case, there will not be enough space on the line for all the
				// characters _after_ the last white-space; so we cut the line at this
				// last white-space.

				if (sepPending) {
					os << " "; // Separate from previous word
					sepPending = false;
				}

				os << string(curLineStart, lastWSpos);

				if (lastWSpos > curLineStart && parserHelpers::isSpace(*(lastWSpos - 1))) {
					state->lastCharIsSpace = true;
				} else {
					state->lastCharIsSpace = false;
				}

				curLineStart = lastWSpos + 1;

				if (flags & text::NO_NEW_LINE_SEQUENCE) {

					os << CRLF;
					curLineLength = 0;

					state->lastCharIsSpace = true;

				} else if (*lastWSpos == '\t') {

					os << CRLF;  // the tab starts the next line
					curLineLength = 0;
					curLineStart = lastWSpos;

					state->lastCharIsSpace = true;

				} else {

					os << NEW_LINE_SEQUENCE;
					curLineLength = NEW_LINE_SEQUENCE_LENGTH;

					state->lastCharIsSpace = true;
				}

				p = curLineStart;
				lastWSpos = lastRunEnd = lastTab = lastBackslash = end;
				newLine = true;
			}
		}

		state->prevWordIsEncoded = false;
		state->lastCharIsBackslash = (!buffer.empty() && buffer[buffer.length() - 1] == '\\');

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
	} else {

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

		// RFC 2047 §2: an encoded-word may not be more than 75 characters long
		const size_t maxEncodedWordLength = 75;

		// Checks whether remaining space on this line is usable. If too few
		// characters can be encoded, start a new line.
		bool startNewLine = true;

		if (curLineLength + 2 < maxLineLength2) {

			const size_t remainingSpaceOnLine = maxLineLength2 - curLineLength - 2;

			if (remainingSpaceOnLine < minWordLength + 10) {

				// Space for no more than 10 encoded chars!
				// It is not worth while to continue on this line...
				startNewLine = true;

			} else {

				// OK, there is enough usable space on the current line.
				startNewLine = false;
			}
		}

		// White-space after an unencoded word is displayed (RFC 2047 section
		// 6.2), so the fold or space added here stands for a leading space
		// or tab, which is then written as is
		const bool leadingWSP =
			!state->isFirstWord && !state->prevWordIsEncoded && isWSP(m_buffer[0]);
		const char sep = leadingWSP ? m_buffer[0] : ' ';

		bool sepAdded = false;

		if (startNewLine) {

			os << CRLF << sep;
			curLineLength = NEW_LINE_SEQUENCE_LENGTH;

			state->lastCharIsSpace = true;
			sepAdded = true;
		}

		// Encode and fold input buffer
		if (!startNewLine && !state->isFirstWord && !state->lastCharIsSpace) {

			os << sep; // Separate from previous word
			++curLineLength;

			state->lastCharIsSpace = true;
			sepAdded = true;
		}

		if (sepAdded && leadingWSP) {
			wordEnc.getNextChunk(1);
		}

		const bool infiniteLength = (ctx.getMaxLineLength() == lineLengthLimits::infinite);

		for (unsigned int i = 0 ; ; ++i) {

			// Compute the number of encoded chars that will fit on this line
			const size_t used = (i == 0 ? curLineLength : NEW_LINE_SEQUENCE_LENGTH);
			const size_t lineRoom = (maxLineLength2 > used) ? maxLineLength2 - used : 0;
			const size_t wordRoom = std::min(lineRoom, maxEncodedWordLength);
			const size_t fit = (wordRoom > minWordLength) ? wordRoom - minWordLength : 1;

			// Get the next encoded chunk
			const string chunk = wordEnc.getNextChunk(fit);

			if (chunk.empty()) {
				break;
			}

			// Start a new encoded word, on the same line if there is no limit
			if (i != 0 && infiniteLength) {

				os << " ";
				++curLineLength;

			} else if (i != 0) {

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
			state->lastCharIsBackslash = false;
		}
	}

	if (newLinePos) {
		*newLinePos = curLineLength;
	}

	state->isFirstWord = false;
}


word& word::operator=(const word& w) {

	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
	m_lang = w.m_lang;

	return *this;
}


word& word::operator=(const string& s) {

	m_buffer = s;
	m_charset = charset::getLocalCharset();
	m_lang.clear();

	return *this;
}


void word::copyFrom(const component& other) {

	const word& w = dynamic_cast <const word&>(other);

	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
	m_lang = w.m_lang;
}


bool word::operator==(const word& w) const {

	return m_charset == w.m_charset && m_buffer == w.m_buffer && m_lang == w.m_lang;
}


bool word::operator!=(const word& w) const {

	return m_charset != w.m_charset || m_buffer != w.m_buffer || m_lang != w.m_lang;
}


bool word::isEquivalent(const word& other) const {

	return getConvertedText(charset(charsets::UTF_8)) == other.getConvertedText(charset(charsets::UTF_8));
}


const string word::getConvertedText(
	const charset& dest,
	const charsetConverterOptions& opts
) const {

	if (dest == m_charset) {
		return m_buffer;  // no conversion needed
	}

	string out;

	try {

		charset::convert(m_buffer, out, m_charset, dest, opts);

	} catch (vmime::exceptions::charset_conv_error& e) {

		// Do not fail if charset is not recognized:
		// copy 'word' as raw text
		out = m_buffer;
	}

	return out;
}


shared_ptr <component> word::clone() const {

	return make_shared <word>(m_buffer, m_charset);
}


const charset& word::getCharset() const {

	return m_charset;
}


void word::setCharset(const charset& ch) {

	m_charset = ch;
}


const string word::getLanguage() const {

	return m_lang;
}


void word::setLanguage(const string& lang) {

	m_lang = lang;
}


const string& word::getBuffer() const {

	return m_buffer;
}


string& word::getBuffer() {

	return m_buffer;
}


bool word::isEmpty() const {

	return m_buffer.empty();
}


void word::setBuffer(const string& buffer) {

	m_buffer = buffer;
}


const std::vector <shared_ptr <component> > word::getChildComponents() {

	return std::vector <shared_ptr <component> >();
}


} // vmime
