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

#include "text.hpp"


namespace vmime
{


text::text()
{
}


text::text(const text& t)
{
	operator=(t);
}


text::text(const string& t, const charset& ch)
{
	makeWordsFromText(t, ch, *this);
}


text::text(const string& t)
{
	makeWordsFromText(t, charset::getLocaleCharset(), *this);
}


text::text(const word& w)
{
	append(w);
}


text::~text()
{
	clear();
}


#if VMIME_WIDE_CHAR_SUPPORT

const wstring text::getDecodedText() const
{
	wstring out;

	for (std::vector <word*>::const_iterator i = m_words.begin() ; i != m_words.end() ; ++i)
	{
		out += (*i)->getDecodedText();
	}

	return (out);
}

#endif


void text::append(const word& w)
{
	m_words.push_back(new word(w));
}


void text::insert(const iterator it, const word& w)
{
	m_words.insert(it.m_iterator, new word(w));
}


void text::clear()
{
	free_container(m_words);

	m_words.clear();
}


void text::remove(const iterator it)
{
	delete (*it.m_iterator);
	m_words.erase(it.m_iterator);
}


text& text::operator=(const text& t)
{
	clear();

	for (std::vector <word*>::const_iterator i = t.m_words.begin() ; i != t.m_words.end() ; ++i)
		m_words.push_back(new word(**i));

	return (*this);
}


const bool text::operator==(const text& t) const
{
	if (size() == t.size())
	{
		bool equal = false;

		std::vector <word*>::const_iterator i = m_words.begin();
		std::vector <word*>::const_iterator j = t.m_words.begin();

		for ( ; equal && i != m_words.end() ; ++i, ++j)
			equal = (*i == *j);

		return (equal);
	}

	return (false);
}


const bool text::operator!=(const text& t) const
{
	return !(*this == t);
}


/** Return the text converted into the specified charset.
  * The encoded-words are decoded and then converted in the
  * destination charset.
  *
  * @param dest output charset
  * @return text decoded in the specified charset
  */

const string text::getConvertedText(const charset& dest) const
{
	string out;

	for (std::vector <word*>::const_iterator i = m_words.begin() ; i != m_words.end() ; ++i)
	{
		out += (*i)->getConvertedText(dest);
	}

	return (out);
}


/** Check whether the list of encoded-words is empty.
  *
  * @return true if the list contains no encoded-word, false otherwise
  */

const bool text::empty() const
{
	return (m_words.size() == 0);
}


/** Return the number of encoded-words in the list.
  *
  * @return number of encoded-words
  */

const size_t text::count() const
{
	return (m_words.size());
}


/** Return the number of encoded-words in the list.
  *
  * @return number of encoded-words
  */

const size_t text::size() const
{
	return (m_words.size());
}


/** Return the first encoded-word of the list.
  *
  * @return first encoded-word
  */

word& text::front()
{
	return (*m_words.front());
}


/** Return the first encoded-word of the list.
  *
  * @return first encoded-word
  */

const word& text::front() const
{
	return (*m_words.front());
}


/** Return the last encoded-word of the list.
  *
  * @return last encoded-word
  */

word& text::back()
{
	return (*m_words.back());
}


/** Return the last encoded-word of the list.
  *
  * @return last encoded-word
  */

const word& text::back() const
{
	return (*m_words.back());
}


} // vmime
