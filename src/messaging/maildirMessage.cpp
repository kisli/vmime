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

#include "maildirMessage.hpp"
#include "maildirFolder.hpp"
#include "maildirUtils.hpp"

#include "../exception.hpp"
#include "../platformDependant.hpp"


namespace vmime {
namespace messaging {


maildirMessage::maildirMessage(maildirFolder* folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false)
{
	m_folder->registerMessage(this);
}


maildirMessage::~maildirMessage()
{
	if (m_folder)
		m_folder->unregisterMessage(this);
}


void maildirMessage::onFolderClosed()
{
	m_folder = NULL;
}


const int maildirMessage::getNumber() const
{
	return (m_num);
}


const message::uid maildirMessage::getUniqueId() const
{
	return (m_uid);
}


const int maildirMessage::getSize() const
{
	if (m_size == -1)
		throw exceptions::unfetched_object();

	return (m_size);
}


const bool maildirMessage::isExpunged() const
{
	return (m_expunged);
}


const structure& maildirMessage::getStructure() const
{
	// TODO
}


structure& maildirMessage::getStructure()
{
	// TODO
}


const header& maildirMessage::getHeader() const
{
	// TODO
}


const int maildirMessage::getFlags() const
{
	if (m_flags == FLAG_UNDEFINED)
		throw exceptions::unfetched_object();

	return (m_flags);
}


void maildirMessage::setFlags(const int flags, const int mode)
{
	if (!m_folder)
		throw exceptions::folder_not_found();

	m_folder->setMessageFlags(m_num, m_num, flags, mode);
}


void maildirMessage::extract(utility::outputStream& os,
	progressionListener* progress, const int start, const int length) const
{
	// TODO
}


void maildirMessage::extractPart(const part& p, utility::outputStream& os,
	progressionListener* progress, const int start, const int length) const
{
	// TODO
}


void maildirMessage::fetchPartHeader(part& p)
{
	// TODO
}


void maildirMessage::fetch(maildirFolder* folder, const int options)
{
	if (m_folder != folder)
		throw exceptions::folder_not_found();

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	utility::auto_ptr <utility::file> file = fsf->create(path);

	/*
	TODO: FETCH_ENVELOPE
	TODO: FETCH_STRUCTURE
	TODO: FETCH_CONTENT_INFO
	TODO: FETCH_FULL_HEADER
	*/

	if (options & folder::FETCH_FLAGS)
		m_flags = maildirUtils::extractFlags(path.getLastComponent());

	if (options & folder::FETCH_SIZE)
		m_size = file->length();

	if (options & folder::FETCH_UID)
		m_uid = maildirUtils::extractId(path.getLastComponent()).getBuffer();
}


} // messaging
} // vmime
