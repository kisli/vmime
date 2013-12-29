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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPTag.hpp"


namespace vmime {
namespace net {
namespace imap {


const int IMAPTag::sm_maxNumber = 52 * 10 * 10 * 10;


IMAPTag::IMAPTag(const int number)
	: m_number(number)
{
	m_tag.resize(4);
	generate();
}


IMAPTag::IMAPTag(const IMAPTag& tag)
	: object(), m_number(tag.m_number)
{
	m_tag.resize(4);
	generate();
}


IMAPTag::IMAPTag()
	: m_number(1)
{
	m_tag.resize(4);
	generate();
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


int IMAPTag::maximumNumber() const
{
	return sm_maxNumber - 1;
}


int IMAPTag::number() const
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
	m_tag[1] = static_cast <char>('0' + (m_number % 1000) / 100);
	m_tag[2] = static_cast <char>('0' + (m_number % 100) / 10);
	m_tag[3] = static_cast <char>('0' + m_number % 10);
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

