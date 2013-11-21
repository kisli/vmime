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


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/message.hpp"

#include <sstream>


namespace vmime {
namespace net {


shared_ptr <const messagePart> messagePart::getPartAt(const size_t pos) const
{
	return getStructure()->getPartAt(pos);
}


shared_ptr <messagePart> messagePart::getPartAt(const size_t pos)
{
	return getStructure()->getPartAt(pos);
}


size_t messagePart::getPartCount() const
{
	return getStructure()->getPartCount();
}



// message::uid


message::uid::uid()
{
}


message::uid::uid(const string& uid)
	: m_str(uid)
{
}


message::uid::uid(const unsigned long uid)
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic());
	oss << uid;

	m_str = oss.str();
}


message::uid::uid(const char* uid)
	: m_str(uid)
{
}


message::uid::uid(const uid& other)
{
	m_str = other.m_str;
}


message::uid& message::uid::operator=(const uid& other)
{
	m_str = other.m_str;
	return *this;
}


message::uid& message::uid::operator=(const string& uid)
{
	m_str = uid;
	return *this;
}


message::uid& message::uid::operator=(const unsigned long uid)
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic());
	oss << uid;

	m_str = oss.str();

	return *this;
}


message::uid::operator string() const
{
	return m_str;
}


bool message::uid::empty() const
{
	return m_str.empty();
}


bool message::uid::operator==(const uid& other) const
{
	return m_str == other.m_str;
}


std::ostream& operator<<(std::ostream& os, const message::uid& uid)
{
	os << static_cast <string>(uid);
	return os;
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

