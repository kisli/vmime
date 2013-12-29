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
#include "vmime/utility/stringUtils.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirMessage::maildirMessage(shared_ptr <maildirFolder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_structure(null)
{
	folder->registerMessage(this);
}


maildirMessage::~maildirMessage()
{
	shared_ptr <maildirFolder> folder = m_folder.lock();

	if (folder)
		folder->unregisterMessage(this);
}


void maildirMessage::onFolderClosed()
{
	m_folder.reset();
}


int maildirMessage::getNumber() const
{
	return (m_num);
}


const message::uid maildirMessage::getUID() const
{
	return (m_uid);
}


size_t maildirMessage::getSize() const
{
	if (m_size == static_cast <size_t>(-1))
		throw exceptions::unfetched_object();

	return (m_size);
}


bool maildirMessage::isExpunged() const
{
	return (m_expunged);
}


shared_ptr <const messageStructure> maildirMessage::getStructure() const
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


shared_ptr <messageStructure> maildirMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


shared_ptr <const header> maildirMessage::getHeader() const
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
	shared_ptr <maildirFolder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::folder_not_found();

	folder->setMessageFlags(messageSet::byNumber(m_num), flags, mode);
}


void maildirMessage::extract(utility::outputStream& os,
	utility::progressListener* progress, const size_t start,
	const size_t length, const bool peek) const
{
	extractImpl(os, progress, 0, m_size, start, length, peek);
}


void maildirMessage::extractPart(shared_ptr <const messagePart> p, utility::outputStream& os,
	utility::progressListener* progress, const size_t start,
	const size_t length, const bool peek) const
{
	shared_ptr <const maildirMessagePart> mp = dynamicCast <const maildirMessagePart>(p);

	extractImpl(os, progress, mp->getBodyParsedOffset(), mp->getBodyParsedLength(),
		start, length, peek);
}


void maildirMessage::extractImpl(utility::outputStream& os, utility::progressListener* progress,
	const size_t start, const size_t length, const size_t partialStart, const size_t partialLength,
	const bool /* peek */) const
{
	shared_ptr <const maildirFolder> folder = m_folder.lock();

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	shared_ptr <utility::file> file = fsf->create(path);

	shared_ptr <utility::fileReader> reader = file->getFileReader();
	shared_ptr <utility::inputStream> is = reader->getInputStream();

	is->skip(start + partialStart);

	byte_t buffer[8192];
	size_t remaining = (partialLength == static_cast <size_t>(-1)
		? length : std::min(partialLength, length));

	const size_t total = remaining;
	size_t current = 0;

	if (progress)
		progress->start(total);

	while (!is->eof() && remaining > 0)
	{
		const size_t read = is->read(buffer, std::min(remaining, sizeof(buffer)));

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


void maildirMessage::fetchPartHeader(shared_ptr <messagePart> p)
{
	shared_ptr <maildirFolder> folder = m_folder.lock();

	shared_ptr <maildirMessagePart> mp = dynamicCast <maildirMessagePart>(p);

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	shared_ptr <utility::file> file = fsf->create(path);

	shared_ptr <utility::fileReader> reader = file->getFileReader();
	shared_ptr <utility::inputStream> is = reader->getInputStream();

	is->skip(mp->getHeaderParsedOffset());

	byte_t buffer[1024];
	size_t remaining = mp->getHeaderParsedLength();

	string contents;
	contents.reserve(remaining);

	while (!is->eof() && remaining > 0)
	{
		const size_t read = is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;

		vmime::utility::stringUtils::appendBytesToString(contents, buffer, read);
	}

	mp->getOrCreateHeader().parse(contents);
}


void maildirMessage::fetch(shared_ptr <maildirFolder> msgFolder, const fetchAttributes& options)
{
	shared_ptr <maildirFolder> folder = m_folder.lock();

	if (folder != msgFolder)
		throw exceptions::folder_not_found();

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	shared_ptr <utility::file> file = fsf->create(path);

	if (options.has(fetchAttributes::FLAGS))
		m_flags = maildirUtils::extractFlags(path.getLastComponent());

	if (options.has(fetchAttributes::SIZE))
		m_size = file->getLength();

	if (options.has(fetchAttributes::UID))
		m_uid = maildirUtils::extractId(path.getLastComponent()).getBuffer();

	if (options.has(fetchAttributes::ENVELOPE | fetchAttributes::CONTENT_INFO |
	                fetchAttributes::FULL_HEADER | fetchAttributes::STRUCTURE |
	                fetchAttributes::IMPORTANCE))
	{
		string contents;

		shared_ptr <utility::fileReader> reader = file->getFileReader();
		shared_ptr <utility::inputStream> is = reader->getInputStream();

		// Need whole message contents for structure
		if (options.has(fetchAttributes::STRUCTURE))
		{
			byte_t buffer[16384];

			contents.reserve(file->getLength());

			while (!is->eof())
			{
				const size_t read = is->read(buffer, sizeof(buffer));
				vmime::utility::stringUtils::appendBytesToString(contents, buffer, read);
			}
		}
		// Need only header
		else
		{
			byte_t buffer[1024];

			contents.reserve(4096);

			while (!is->eof())
			{
				const size_t read = is->read(buffer, sizeof(buffer));
				vmime::utility::stringUtils::appendBytesToString(contents, buffer, read);

				const size_t sep1 = contents.rfind("\r\n\r\n");
				const size_t sep2 = contents.rfind("\n\n");

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
		if (options.has(fetchAttributes::STRUCTURE))
		{
			m_structure = make_shared <maildirMessageStructure>(shared_ptr <maildirMessagePart>(), msg);
		}

		// Extract some header fields or whole header
		if (options.has(fetchAttributes::ENVELOPE |
		                fetchAttributes::CONTENT_INFO |
		                fetchAttributes::FULL_HEADER |
		                fetchAttributes::IMPORTANCE))
		{
			getOrCreateHeader()->copyFrom(*(msg.getHeader()));
		}
	}
}


shared_ptr <header> maildirMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (m_header);
	else
		return (m_header = make_shared <header>());
}


shared_ptr <vmime::message> maildirMessage::getParsedMessage()
{
	std::ostringstream oss;
	utility::outputStreamAdapter os(oss);

	extract(os);

	shared_ptr <vmime::message> msg = make_shared <vmime::message>();
	msg->parse(oss.str());

	return msg;
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

