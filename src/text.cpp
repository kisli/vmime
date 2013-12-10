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

#include "vmime/text.hpp"

#include "vmime/parserHelpers.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


text::text()
{
}


text::text(const text& t)
	: headerFieldValue()
{
	copyFrom(t);
}


text::text(const string& t, const charset& ch)
{
	createFromString(t, ch);
}


text::text(const string& t)
{
	createFromString(t, charset::getLocalCharset());
}


text::text(const word& w)
{
	appendWord(make_shared <word>(w));
}


text::~text()
{
	removeAllWords();
}


void text::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	removeAllWords();

	size_t newPos;

	const std::vector <shared_ptr <word> > words = word::parseMultiple(ctx, buffer, position, end, &newPos);

	copy_vector(words, m_words);

	setParsedBounds(position, newPos);

	if (newPosition)
		*newPosition = newPos;
}


void text::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	encodeAndFold(ctx, os, curLinePos, newLinePos, 0);
}


void text::copyFrom(const component& other)
{
	const text& t = dynamic_cast <const text&>(other);

	removeAllWords();

	for (std::vector <shared_ptr <word> >::const_iterator i = t.m_words.begin() ; i != t.m_words.end() ; ++i)
		m_words.push_back(make_shared <word>(**i));
}


text& text::operator=(const component& other)
{
	copyFrom(other);
	return (*this);
}


text& text::operator=(const text& other)
{
	copyFrom(other);
	return (*this);
}


bool text::operator==(const text& t) const
{
	if (getWordCount() == t.getWordCount())
	{
		bool equal = true;

		std::vector <shared_ptr <word> >::const_iterator i = m_words.begin();
		std::vector <shared_ptr <word> >::const_iterator j = t.m_words.begin();

		for ( ; equal && i != m_words.end() ; ++i, ++j)
			equal = (**i == **j);

		return (equal);
	}

	return (false);
}


bool text::operator!=(const text& t) const
{
	return !(*this == t);
}


const string text::getConvertedText(const charset& dest, const charsetConverterOptions& opts) const
{
	string out;

	for (std::vector <shared_ptr <word> >::const_iterator i = m_words.begin() ; i != m_words.end() ; ++i)
		out += (*i)->getConvertedText(dest, opts);

	return (out);
}


void text::appendWord(shared_ptr <word> w)
{
	m_words.push_back(w);
}


void text::insertWordBefore(const size_t pos, shared_ptr <word> w)
{
	m_words.insert(m_words.begin() + pos, w);
}


void text::insertWordAfter(const size_t pos, shared_ptr <word> w)
{
	m_words.insert(m_words.begin() + pos + 1, w);
}


void text::removeWord(const size_t pos)
{
	const std::vector <shared_ptr <word> >::iterator it = m_words.begin() + pos;

	m_words.erase(it);
}


void text::removeAllWords()
{
	m_words.clear();
}


size_t text::getWordCount() const
{
	return (m_words.size());
}


bool text::isEmpty() const
{
	return (m_words.empty());
}


const shared_ptr <word> text::getWordAt(const size_t pos)
{
	return (m_words[pos]);
}


const shared_ptr <const word> text::getWordAt(const size_t pos) const
{
	return (m_words[pos]);
}


const std::vector <shared_ptr <const word> > text::getWordList() const
{
	std::vector <shared_ptr <const word> > list;

	list.reserve(m_words.size());

	for (std::vector <shared_ptr <word> >::const_iterator it = m_words.begin() ;
	     it != m_words.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <word> > text::getWordList()
{
	return (m_words);
}


shared_ptr <component> text::clone() const
{
	return make_shared <text>(*this);
}


shared_ptr <text> text::newFromString(const string& in, const charset& ch)
{
	shared_ptr <text> t = make_shared <text>();

	t->createFromString(in, ch);

	return t;
}


void text::createFromString(const string& in, const charset& ch)
{
	size_t asciiCount = 0;
	size_t asciiPercent = 0;

	removeAllWords();

	// Check whether there is a recommended encoding for this charset.
	// If so, the whole buffer will be encoded. Else, the number of
	// 7-bit (ASCII) bytes in the input will be used to determine if
	// we need to encode the whole buffer.
	encoding recommendedEnc;
	const bool alwaysEncode = ch.getRecommendedEncoding(recommendedEnc);

	if (!alwaysEncode)
	{
		asciiCount = utility::stringUtils::countASCIIchars(in.begin(), in.end());
		asciiPercent = (in.length() == 0 ? 100 : (100 * asciiCount) / in.length());
	}

	// If there are "too much" non-ASCII chars, encode everything
	if (alwaysEncode || asciiPercent < 60)  // less than 60% ASCII chars
	{
		appendWord(make_shared <word>(in, ch));
	}
	// Else, only encode words which need it
	else
	{
		bool is8bit = false;     // is the current word 8-bit?
		bool prevIs8bit = false; // is previous word 8-bit?
		unsigned int count = 0;  // total number of words

		for (size_t end = in.size(), pos = 0, start = 0 ; ; )
		{
			if (pos == end || parserHelpers::isSpace(in[pos]))
			{
				const string chunk(in.begin() + start, in.begin() + pos);

				if (pos != end)
					++pos;

				if (is8bit)
				{
					if (count && prevIs8bit)
					{
						// No need to create a new encoded word, just append
						// the current word to the previous one.
						shared_ptr <word> w = getWordAt(getWordCount() - 1);
						w->getBuffer() += " " + chunk;
					}
					else
					{
						if (count)
						{
							shared_ptr <word> w = getWordAt(getWordCount() - 1);
							w->getBuffer() += ' ';
						}

						appendWord(make_shared <word>(chunk, ch));

						prevIs8bit = true;
						++count;
					}
				}
				else
				{
					if (count && !prevIs8bit)
					{
						shared_ptr <word> w = getWordAt(getWordCount() - 1);
						w->getBuffer() += " " + chunk;
					}
					else
					{
						appendWord(make_shared <word>
							(chunk, charset(charsets::US_ASCII)));

						prevIs8bit = false;
						++count;
					}
				}

				if (pos == end)
					break;

				is8bit = false;
				start = pos;
			}
			else if (!parserHelpers::isAscii(in[pos]))
			{
				is8bit = true;
				++pos;
			}
			else
			{
				++pos;
			}
		}
	}
}


void text::encodeAndFold
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t firstLineOffset, size_t* lastLineLength, const int flags) const
{
	size_t curLineLength = firstLineOffset;
	word::generatorState state;

	for (size_t wi = 0 ; wi < getWordCount() ; ++wi)
	{
		getWordAt(wi)->generate(ctx, os, curLineLength,
			&curLineLength, flags, &state);
	}

	if (lastLineLength)
		*lastLineLength = curLineLength;
}


shared_ptr <text> text::decodeAndUnfold(const string& in)
{
	shared_ptr <text> t = make_shared <text>();

	decodeAndUnfold(parsingContext::getDefaultContext(), in, t.get());

	return t;
}


shared_ptr <text> text::decodeAndUnfold(const parsingContext& ctx, const string& in)
{
	shared_ptr <text> t = make_shared <text>();

	decodeAndUnfold(ctx, in, t.get());

	return t;
}


text* text::decodeAndUnfold(const string& in, text* generateInExisting)
{
	return decodeAndUnfold(parsingContext::getDefaultContext(), in, generateInExisting);
}


text* text::decodeAndUnfold(const parsingContext& ctx, const string& in, text* generateInExisting)
{
	text* out = (generateInExisting != NULL) ? generateInExisting : new text();

	out->removeAllWords();

	const std::vector <shared_ptr <word> > words = word::parseMultiple(ctx, in, 0, in.length(), NULL);

	copy_vector(words, out->m_words);

	return (out);
}


const std::vector <shared_ptr <component> > text::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	copy_vector(m_words, list);

	return (list);
}


const string text::getWholeBuffer() const
{
	string res;

	for (std::vector <shared_ptr <word> >::const_iterator it = m_words.begin() ;
	     it != m_words.end() ; ++it)
	{
		res += (*it)->getBuffer();
	}

	return res;
}


} // vmime
