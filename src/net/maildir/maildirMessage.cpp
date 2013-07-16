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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/maildir/maildirMessage.hpp"
#include "vmime/net/maildir/maildirMessagePart.hpp"
#include "vmime/net/maildir/maildirMessageStructure.hpp"
#include "vmime/net/maildir/maildirFolder.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"
#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/message.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirMessage::maildirMessage(ref <maildirFolder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_structure(NULL)
{
	folder->registerMessage(this);
}


maildirMessage::~maildirMessage()
{
	ref <maildirFolder> folder = m_folder.acquire();

	if (folder)
		folder->unregisterMessage(this);
}


void maildirMessage::onFolderClosed()
{
	m_folder = NULL;
}


int maildirMessage::getNumber() const
{
	return (m_num);
}


const message::uid maildirMessage::getUID() const
{
	return (m_uid);
}


int maildirMessage::getSize() const
{
	if (m_size == -1)
		throw exceptions::unfetched_object();

	return (m_size);
}


bool maildirMessage::isExpunged() const
{
	return (m_expunged);
}


ref <const messageStructure> maildirMessage::getStructure() const
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <messageStructure> maildirMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <const header> maildirMessage::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


int maildirMessage::getFlags() const
{
	if (m_flags == FLAG_UNDEFINED)
		throw exceptions::unfetched_object();

	return (m_flags);
}


void maildirMessage::setFlags(const int flags, const int mode)
{
	ref <maildirFolder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::folder_not_found();

	folder->setMessageFlags(m_num, m_num, flags, mode);
}


void maildirMessage::extract(utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool peek) const
{
	extractImpl(os, progress, 0, m_size, start, length, peek);
}


void maildirMessage::extractPart(ref <const messagePart> p, utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool peek) const
{
	ref <const maildirMessagePart> mp = p.dynamicCast <const maildirMessagePart>();

	extractImpl(os, progress, mp->getBodyParsedOffset(), mp->getBodyParsedLength(),
		start, length, peek);
}


void maildirMessage::extractImpl(utility::outputStream& os, utility::progressListener* progress,
	const int start, const int length, const int partialStart, const int partialLength,
	const bool /* peek */) const
{
	ref <const maildirFolder> folder = m_folder.acquire();

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	ref <utility::fileReader> reader = file->getFileReader();
	ref <utility::inputStream> is = reader->getInputStream();

	is->skip(start + partialStart);

	utility::stream::value_type buffer[8192];
	utility::stream::size_type remaining = (partialLength == -1 ? length
		: std::min(partialLength, length));

	const int total = remaining;
	int current = 0;

	if (progress)
		progress->start(total);

	while (!is->eof() && remaining > 0)
	{
		const utility::stream::size_type read =
			is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;
		current += read;

		os.write(buffer, read);

		if (progress)
			progress->progress(current, total);
	}

	if (progress)
		progress->stop(total);

	// TODO: mark as read unless 'peek' is set
}


void maildirMessage::fetchPartHeader(ref <messagePart> p)
{
	ref <maildirFolder> folder = m_folder.acquire();

	ref <maildirMessagePart> mp = p.dynamicCast <maildirMessagePart>();

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	ref <utility::fileReader> reader = file->getFileReader();
	ref <utility::inputStream> is = reader->getInputStream();

	is->skip(mp->getHeaderParsedOffset());

	utility::stream::value_type buffer[1024];
	utility::stream::size_type remaining = mp->getHeaderParsedLength();

	string contents;
	contents.reserve(remaining);

	while (!is->eof() && remaining > 0)
	{
		const utility::stream::size_type read =
			is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;

		contents.append(buffer, read);
	}

	mp->getOrCreateHeader().parse(contents);
}


void maildirMessage::fetch(ref <maildirFolder> msgFolder, const int options)
{
	ref <maildirFolder> folder = m_folder.acquire();

	if (folder != msgFolder)
		throw exceptions::folder_not_found();

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	if (options & folder::FETCH_FLAGS)
		m_flags = maildirUtils::extractFlags(path.getLastComponent());

	if (options & folder::FETCH_SIZE)
		m_size = file->getLength();

	if (options & folder::FETCH_UID)
		m_uid = maildirUtils::extractId(path.getLastComponent()).getBuffer();

	if (options & (folder::FETCH_ENVELOPE | folder::FETCH_CONTENT_INFO |
	               folder::FETCH_FULL_HEADER | folder::FETCH_STRUCTURE |
	               folder::FETCH_IMPORTANCE))
	{
		string contents;

		ref <utility::fileReader> reader = file->getFileReader();
		ref <utility::inputStream> is = reader->getInputStream();

		// Need whole message contents for structure
		if (options & folder::FETCH_STRUCTURE)
		{
			utility::stream::value_type buffer[16384];

			contents.reserve(file->getLength());

			while (!is->eof())
			{
				const utility::stream::size_type read = is->read(buffer, sizeof(buffer));
				contents.append(buffer, read);
			}
		}
		// Need only header
		else
		{
			utility::stream::value_type buffer[1024];

			contents.reserve(4096);

			while (!is->eof())
			{
				const utility::stream::size_type read = is->read(buffer, sizeof(buffer));
				contents.append(buffer, read);

				const string::size_type sep1 = contents.rfind("\r\n\r\n");
				const string::size_type sep2 = contents.rfind("\n\n");

				if (sep1 != string::npos)
				{
					contents.erase(contents.begin() + sep1 + 4, contents.end());
					break;
				}
				else if (sep2 != string::npos)
				{
					contents.erase(contents.begin() + sep2 + 2, contents.end());
					break;
				}
			}
		}

		vmime::message msg;
		msg.parse(contents);

		// Extract structure
		if (options & folder::FETCH_STRUCTURE)
		{
			m_structure = vmime::create <maildirMessageStructure>(null, msg);
		}

		// Extract some header fields or whole header
		if (options & (folder::FETCH_ENVELOPE |
		               folder::FETCH_CONTENT_INFO |
		               folder::FETCH_FULL_HEADER |
		               folder::FETCH_IMPORTANCE))
		{
			getOrCreateHeader()->copyFrom(*(msg.getHeader()));
		}
	}
}


ref <header> maildirMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (m_header);
	else
		return (m_header = vmime::create <header>());
}


ref <vmime::message> maildirMessage::getParsedMessage()
{
	std::ostringstream oss;
	utility::outputStreamAdapter os(oss);

	extract(os);

	vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
	msg->parse(oss.str());

	return msg;
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

