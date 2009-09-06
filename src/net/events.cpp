//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/events.hpp"
#include "vmime/net/folder.hpp"

#include <algorithm>


namespace vmime {
namespace net {
namespace events {


//
// messageCountEvent
//

messageCountEvent::messageCountEvent
	(ref <folder> folder, const Types type, const std::vector <int>& nums)
		: m_folder(folder), m_type(type)
{
	m_nums.resize(nums.size());
	std::copy(nums.begin(), nums.end(), m_nums.begin());
}


ref <folder> messageCountEvent::getFolder() const { return (m_folder); }
messageCountEvent::Types messageCountEvent::getType() const { return (m_type); }
const std::vector <int>& messageCountEvent::getNumbers() const { return (m_nums); }


void messageCountEvent::dispatch(messageCountListener* listener) const
{
	if (m_type == TYPE_ADDED)
		listener->messagesAdded(*this);
	else
		listener->messagesRemoved(*this);
}


//
// messageChangedEvent
//

messageChangedEvent::messageChangedEvent
	(ref <folder> folder, const Types type, const std::vector <int>& nums)
		: m_folder(folder), m_type(type)
{
	m_nums.resize(nums.size());
	std::copy(nums.begin(), nums.end(), m_nums.begin());
}


ref <folder> messageChangedEvent::getFolder() const { return (m_folder); }
messageChangedEvent::Types messageChangedEvent::getType() const { return (m_type); }
const std::vector <int>& messageChangedEvent::getNumbers() const { return (m_nums); }


void messageChangedEvent::dispatch(messageChangedListener* listener) const
{
	listener->messageChanged(*this);
}


//
// folderEvent
//

folderEvent::folderEvent
	(ref <folder> folder, const Types type,
	 const utility::path& oldPath, const utility::path& newPath)
	: m_folder(folder), m_type(type), m_oldPath(oldPath), m_newPath(newPath)
{
}


ref <folder> folderEvent::getFolder() const { return (m_folder); }
folderEvent::Types folderEvent::getType() const { return (m_type); }


void folderEvent::dispatch(folderListener* listener) const
{
	switch (m_type)
	{
	case TYPE_CREATED: listener->folderCreated(*this); break;
	case TYPE_RENAMED: listener->folderRenamed(*this); break;
	case TYPE_DELETED: listener->folderDeleted(*this); break;
	}
}


} // events
} // net
} // vmime
