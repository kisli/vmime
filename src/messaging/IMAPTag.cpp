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

#include "vmime/messaging/IMAPTag.hpp"


namespace vmime {
namespace messaging {


const int IMAPTag::sm_maxNumber = 52 * 10 * 10 * 10;


IMAPTag::IMAPTag(const int number)
	: m_number(number)
{
	m_tag.resize(4);
}


IMAPTag::IMAPTag(const IMAPTag& tag)
	: m_number(tag.m_number)
{
	m_tag.resize(4);
}


IMAPTag::IMAPTag()
	: m_number(0)
{
	m_tag.resize(4);
}


IMAPTag& IMAPTag::operator++()
{
	++m_number;

	if (m_number >= sm_maxNumber)
		m_number = 1;

	generate();

	return (*this);
}


const IMAPTag IMAPTag::operator++(int)
{
	IMAPTag old(*this);
	operator++();
	return (old);
}


const int IMAPTag::number() const
{
	return (m_number);
}


IMAPTag::operator string() const
{
	return (m_tag);
}


void IMAPTag::generate()
{
	static const char prefixChars[53] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	m_tag[0] = prefixChars[m_number / 1000];
	m_tag[1] = '0' + (m_number % 1000) / 100;
	m_tag[2] = '0' + (m_number % 100) / 10;
	m_tag[3] = '0' + (m_number % 10);
}


} // messaging
} // vmime
