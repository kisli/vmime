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


namespace vmime {
namespace messaging {


maildirMessage::maildirMessage(maildirFolder* folder, const int num)
	: m_folder(folder), m_num(num)
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
}


const message::uid maildirMessage::getUniqueId() const
{
}


const int maildirMessage::getSize() const
{
}


const bool maildirMessage::isExpunged() const
{
}


const structure& maildirMessage::getStructure() const
{
}


structure& maildirMessage::getStructure()
{
}


const header& maildirMessage::getHeader() const
{
}


const int maildirMessage::getFlags() const
{
}


void maildirMessage::setFlags(const int flags, const int mode)
{
}


void maildirMessage::extract(utility::outputStream& os, progressionListener* progress, const int start, const int length) const
{
}


void maildirMessage::extractPart(const part& p, utility::outputStream& os, progressionListener* progress, const int start, const int length) const
{
}


void maildirMessage::fetchPartHeader(part& p)
{
}


} // messaging
} // vmime
