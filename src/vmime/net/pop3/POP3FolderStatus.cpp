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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3FolderStatus.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3FolderStatus::POP3FolderStatus()
	: m_count(0),
	  m_unseen(0)
{
}


POP3FolderStatus::POP3FolderStatus(const POP3FolderStatus& other)
	: folderStatus(),
	  m_count(other.m_count),
	  m_unseen(other.m_unseen)
{
}


unsigned int POP3FolderStatus::getMessageCount() const
{
	return m_count;
}


unsigned int POP3FolderStatus::getUnseenCount() const
{
	return m_unseen;
}


void POP3FolderStatus::setMessageCount(const unsigned int count)
{
	m_count = count;
}


void POP3FolderStatus::setUnseenCount(const unsigned int unseen)
{
	m_unseen = unseen;
}


shared_ptr <folderStatus> POP3FolderStatus::clone() const
{
	return make_shared <POP3FolderStatus>(*this);
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3
