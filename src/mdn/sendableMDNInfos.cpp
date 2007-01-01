//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2007 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/mdn/sendableMDNInfos.hpp"


namespace vmime {
namespace mdn {


sendableMDNInfos::sendableMDNInfos(const ref <const message> msg, const mailbox& mbox)
	: m_msg(msg), m_mailbox(mbox)
{
}


sendableMDNInfos::sendableMDNInfos(const sendableMDNInfos& other)
	: MDNInfos()
{
	copyFrom(other);
}


sendableMDNInfos& sendableMDNInfos::operator=(const sendableMDNInfos& other)
{
	copyFrom(other);
	return (*this);
}


const ref <const message> sendableMDNInfos::getMessage() const
{
	return (m_msg);
}


const mailbox& sendableMDNInfos::getRecipient() const
{
	return (m_mailbox);
}


void sendableMDNInfos::copyFrom(const sendableMDNInfos& other)
{
	m_msg = other.m_msg;
	m_mailbox = other.m_mailbox;
}


} // mdn
} // vmime
