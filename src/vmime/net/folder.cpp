//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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


#include "vmime/net/folder.hpp"

#include <algorithm>


namespace vmime {
namespace net {


int folder::getType() {

	return getAttributes().getType();
}


int folder::getFlags() {

	return getAttributes().getFlags();
}


void folder::addMessageChangedListener(events::messageChangedListener* l) {

	m_messageChangedListeners.push_back(l);
}


void folder::removeMessageChangedListener(events::messageChangedListener* l) {

	m_messageChangedListeners.erase(
		std::remove(m_messageChangedListeners.begin(), m_messageChangedListeners.end(), l),
		m_messageChangedListeners.end()
	);
}


void folder::notifyMessageChanged(const shared_ptr <events::messageChangedEvent>& event) {

	for (std::list <events::messageChangedListener*>::iterator
	     it = m_messageChangedListeners.begin() ; it != m_messageChangedListeners.end() ; ++it) {

		event->dispatch(*it);
	}
}


void folder::addMessageCountListener(events::messageCountListener* l) {

	m_messageCountListeners.push_back(l);
}


void folder::removeMessageCountListener(events::messageCountListener* l) {

	m_messageCountListeners.erase(
		std::remove(m_messageCountListeners.begin(), m_messageCountListeners.end(), l),
		m_messageCountListeners.end()
	);
}


void folder::notifyMessageCount(const shared_ptr <events::messageCountEvent>& event) {

	for (std::list <events::messageCountListener*>::iterator
	     it = m_messageCountListeners.begin() ; it != m_messageCountListeners.end() ; ++it) {

		event->dispatch(*it);
	}
}


void folder::addFolderListener(events::folderListener* l) {

	m_folderListeners.push_back(l);
}


void folder::removeFolderListener(events::folderListener* l) {

	m_folderListeners.erase(
		std::remove(m_folderListeners.begin(), m_folderListeners.end(), l),
		m_folderListeners.end()
	);
}


void folder::notifyFolder(const shared_ptr <events::folderEvent>& event) {

	for (std::list <events::folderListener*>::iterator
	     it = m_folderListeners.begin() ; it != m_folderListeners.end() ; ++it) {

		event->dispatch(*it);
	}
}


void folder::notifyEvent(const shared_ptr <events::event>& event) {

	if (event->getClass() == events::messageCountEvent::EVENT_CLASS) {
		notifyMessageCount(dynamicCast <events::messageCountEvent>(event));
	} else if (event->getClass() == events::messageChangedEvent::EVENT_CLASS) {
		notifyMessageChanged(dynamicCast <events::messageChangedEvent>(event));
	} else if (event->getClass() == events::folderEvent::EVENT_CLASS) {
		notifyFolder(dynamicCast <events::folderEvent>(event));
	}
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

