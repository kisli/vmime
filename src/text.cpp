//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/text.hpp"

#include "vmime/parserHelpers.hpp"


namespace vmime
{


text::text()
{
}


text::text(const text& t)
	: component()
{
	copyFrom(t);
}


text::text(const string& t, const charset& ch)
{
	text::newFromString(t, ch, this);
}


text::text(const string& t)
{
	text::newFromString(t, charset::getLocaleCharset(), this);
}


text::text(const word& w)
{
	appendWord(new word(w));
}


text::~text()
{
	removeAllWords();
}


void text::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	removeAllWords();

	string::size_type newPos;

	const std::vector <word*> words = word::parseMultiple(buffer, position, end, &newPos);

	copy_vector(words, m_words);

	setParsedBounds(position, newPos);

	if (newPosition)
		*newPosition = newPos;
}


void text::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	encodeAndFold(os, maxLineLength, curLinePos, newLinePos, 0);
}


#if VMIME_WIDE_CHAR_SUPPORT

const wstring text::getDecodedText() const
{
	wstring out;

	for (std::vector <word*>::const_iterator i = m_words.begin() ; i != m_words.end() ; ++i)
		out += (*i)->getDecodedText();

	return (out);
}

#endif


void text::copyFrom(const component& other)
{
	const text& t = dynamic_cast <const text&>(other);

	removeAllWords();

	for (std::vector <word*>::const_iterator i = t.m_words.begin() ; i != t.m_words.end() ; ++i)
		m_words.push_back(new word(**i));
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


const bool text::operator==(const text& t) const
{
	if (getWordCount() == t.getWordCount())
	{
		bool equal = true;

		std::vector <word*>::const_iterator i = m_words.begin();
		std::vector <word*>::const_iterator j = t.m_words.begin();

		for ( ; equal && i != m_words.end() ; ++i, ++j)
			equal = (**i == **j);

		return (equal);
	}

	return (false);
}


const bool text::operator!=(const text& t) const
{
	return !(*this == t);
}


const string text::getConvertedText(const charset& dest) const
{
	string out;

	for (std::vector <word*>::const_iterator i = m_words.begin() ; i != m_words.end() ; ++i)
		out += (*i)->getConvertedText(dest);

	return (out);
}


void text::appendWord(word* w)
{
	m_words.push_back(w);
}


void text::insertWordBefore(const int pos, word* w)
{
	m_words.insert(m_words.begin() + pos, w);
}


void text::insertWordAfter(const int pos, word* w)
{
	m_words.insert(m_words.begin() + pos + 1, w);
}


void text::removeWord(const int pos)
{
	const std::vector <word*>::iterator it = m_words.begin() + pos;

	delete (*it);

	m_words.erase(it);
}


void text::removeAllWords()
{
	free_container(m_words);
}


const int text::getWordCount() const
{
	return (m_words.size());
}


const bool text::isEmpty() const
{
	return (m_words.empty());
}


word* text::getWordAt(const int pos)
{
	return (m_words[pos]);
}


const word* text::getWordAt(const int pos) const
{
	return (m_words[pos]);
}


const std::vector <const word*> text::getWordList() const
{
	std::vector <const word*> list;

	list.reserve(m_words.size());

	for (std::vector <word*>::const_iterator it = m_words.begin() ;
	     it != m_words.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <word*> text::getWordList()
{
	return (m_words);
}


text* text::clone() const
{
	return new text(*this);
}


text* text::newFromString(const string& in, const charset& ch, text* generateInExisting)
{
	const string::const_iterator end = in.end();
	string::const_iterator p = in.begin();
	string::const_iterator start = in.begin();

	bool is8bit = false;     // is the current word 8-bit?
	bool prevIs8bit = false; // is previous word 8-bit?
	unsigned int count = 0;  // total number of words

	text* out = (generateInExisting != NULL) ? generateInExisting : new text();

	out->removeAllWords();

	for ( ; ; )
	{
		if (p == end || parserHelpers::isspace(*p))
		{
			if (p != end)
				++p;

			if (is8bit)
			{
				if (count && prevIs8bit)
				{
					// No need to create a new encoded word, just append
					// the current word to the previous one.
					out->getWordAt(out->getWordCount() - 1)->
						getBuffer() += string(start, p);
				}
				else
				{
					out->appendWord(new word(string(start, p), ch));

					prevIs8bit = true;
					++count;
				}
			}
			else
			{
				if (count && !prevIs8bit)
				{
					out->getWordAt(out->getWordCount() - 1)->
						getBuffer() += string(start, p);
				}
				else
				{
					out->appendWord(new word
						(string(start, p), charset(charsets::US_ASCII)));

					prevIs8bit = false;
					++count;
				}
			}

			if (p == end)
				break;

			is8bit = false;
			start = p;
		}
		else if (!parserHelpers::isascii(*p))
		{
			is8bit = true;
			++p;
		}
		else
		{
			++p;
		}
	}

	return (out);
}


void text::encodeAndFold(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type firstLineOffset, string::size_type* lastLineLength, const int flags) const
{
	string::size_type curLineLength = firstLineOffset;

	for (int wi = 0 ; wi < getWordCount() ; ++wi)
	{
		getWordAt(wi)->generate(os, maxLineLength, curLineLength,
			&curLineLength, flags, (wi == 0));
	}

	if (lastLineLength)
		*lastLineLength = curLineLength;
}


text* text::decodeAndUnfold(const string& in, text* generateInExisting)
{
	text* out = (generateInExisting != NULL) ? generateInExisting : new text();

	out->removeAllWords();

	const std::vector <word*> words = word::parseMultiple(in, 0, in.length(), NULL);

	copy_vector(words, out->m_words);

	return (out);
}


const std::vector <const component*> text::getChildComponents() const
{
	std::vector <const component*> list;

	copy_vector(m_words, list);

	return (list);
}


} // vmime
