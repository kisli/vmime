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

#include "vmime/word.hpp"


namespace vmime
{


word::word()
	: m_charset(charset::getLocaleCharset())
{
}


word::word(const word& w)
	: m_buffer(w.m_buffer), m_charset(w.m_charset)
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


#if VMIME_WIDE_CHAR_SUPPORT

const wstring word::getDecodedText() const
{
	wstring out;

	charset::decode(m_buffer, out, m_charset);

	return (out);
}

#endif


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


void word::copyFrom(const word& w)
{
	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
}


const bool word::operator==(const word& w) const
{
	return (m_charset == w.m_charset && m_buffer == w.m_buffer);
}


const bool word::operator!=(const word& w) const
{
	return (m_charset != w.m_charset || m_buffer != w.m_buffer);
}


const string word::getConvertedText(const charset& dest) const
{
	string out;

	charset::convert(m_buffer, out, m_charset, dest);

	return (out);
}


word* word::clone() const
{
	return new word(m_buffer, m_charset);
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


void word::setBuffer(const string& buffer)
{
	m_buffer = buffer;
}


} // vmime
