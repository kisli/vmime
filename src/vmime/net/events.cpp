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


#include "vmime/net/events.hpp"
#include "vmime/net/folder.hpp"

#include <algorithm>


namespace vmime {
namespace net {
namespace events {


//
// event
//

event::event()
{
}


event::~event()
{
}


//
// messageCountEvent
//

const char* messageCountEvent::EVENT_CLASS = "messageCountEvent";


messageCountEvent::messageCountEvent
	(shared_ptr <folder> folder, const Types type, const std::vector <int>& nums)
		: m_folder(folder), m_type(type)
{
	m_nums.resize(nums.size());
	std::copy(nums.begin(), nums.end(), m_nums.begin());
}


shared_ptr <folder> messageCountEvent::getFolder() const { return (m_folder); }
messageCountEvent::Types messageCountEvent::getType() const { return (m_type); }
const std::vector <int>& messageCountEvent::getNumbers() const { return (m_nums); }


void messageCountEvent::dispatch(messageCountListener* listener)
{
	if (m_type == TYPE_ADDED)
		listener->messagesAdded(dynamicCast <messageCountEvent>(shared_from_this()));
	else
		listener->messagesRemoved(dynamicCast <messageCountEvent>(shared_from_this()));
}


const char* messageCountEvent::getClass() const
{
	return EVENT_CLASS;
}


//
// messageChangedEvent
//

const char* messageChangedEvent::EVENT_CLASS = "messageChangedEvent";


messageChangedEvent::messageChangedEvent
	(shared_ptr <folder> folder, const Types type, const std::vector <int>& nums)
		: m_folder(folder), m_type(type)
{
	m_nums.resize(nums.size());
	std::copy(nums.begin(), nums.end(), m_nums.begin());
}


shared_ptr <folder> messageChangedEvent::getFolder() const { return (m_folder); }
messageChangedEvent::Types messageChangedEvent::getType() const { return (m_type); }
const std::vector <int>& messageChangedEvent::getNumbers() const { return (m_nums); }


void messageChangedEvent::dispatch(messageChangedListener* listener)
{
	listener->messageChanged(dynamicCast <messageChangedEvent>(shared_from_this()));
}


const char* messageChangedEvent::getClass() const
{
	return EVENT_CLASS;
}


//
// folderEvent
//

const char* folderEvent::EVENT_CLASS = "folderEvent";


folderEvent::folderEvent
	(shared_ptr <folder> folder, const Types type,
	 const utility::path& oldPath, const utility::path& newPath)
	: m_folder(folder), m_type(type), m_oldPath(oldPath), m_newPath(newPath)
{
}


shared_ptr <folder> folderEvent::getFolder() const { return (m_folder); }
folderEvent::Types folderEvent::getType() const { return (m_type); }


void folderEvent::dispatch(folderListener* listener)
{
	switch (m_type)
	{
	case TYPE_CREATED: listener->folderCreated(dynamicCast <folderEvent>(shared_from_this())); break;
	case TYPE_RENAMED: listener->folderRenamed(dynamicCast <folderEvent>(shared_from_this())); break;
	case TYPE_DELETED: listener->folderDeleted(dynamicCast <folderEvent>(shared_from_this())); break;
	}
}


const char* folderEvent::getClass() const
{
	return EVENT_CLASS;
}


} // events
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

