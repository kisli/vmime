//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/messaging/maildirMessage.hpp"
#include "vmime/messaging/maildirFolder.hpp"
#include "vmime/messaging/maildirUtils.hpp"

#include "vmime/messageParser.hpp"   // to include "message.hpp" in root directory

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"


namespace vmime {
namespace messaging {


//
// maildirPart
//

class maildirStructure;

class maildirPart : public part
{
public:

	maildirPart(maildirPart* parent, const int number, const bodyPart& part);
	~maildirPart();


	const structure& getStructure() const;
	structure& getStructure();

	const maildirPart* getParent() const { return (m_parent); }

	const mediaType& getType() const { return (m_mediaType); }
	const int getSize() const { return (m_size); }
	const int getNumber() const { return (m_number); }

	const header& getHeader() const
	{
		if (m_header == NULL)
			throw exceptions::unfetched_object();
		else
			return (*m_header);
	}

	header& getOrCreateHeader()
	{
		if (m_header != NULL)
			return (*m_header);
		else
			return (*(m_header = new header()));
	}

	const int getHeaderParsedOffset() const { return (m_headerParsedOffset); }
	const int getHeaderParsedLength() const { return (m_headerParsedLength); }

	const int getBodyParsedOffset() const { return (m_bodyParsedOffset); }
	const int getBodyParsedLength() const { return (m_bodyParsedLength); }

private:

	maildirStructure* m_structure;
	maildirPart* m_parent;
	header* m_header;

	int m_number;
	int m_size;
	mediaType m_mediaType;

	int m_headerParsedOffset;
	int m_headerParsedLength;

	int m_bodyParsedOffset;
	int m_bodyParsedLength;
};



//
// maildirStructure
//

class maildirStructure : public structure
{
private:

	maildirStructure()
	{
	}

public:

	maildirStructure(maildirPart* parent, const bodyPart& part)
	{
		m_parts.push_back(new maildirPart(parent, 1, part));
	}

	maildirStructure(maildirPart* parent, const std::vector <const vmime::bodyPart*>& list)
	{
		int number = 1;

		for (unsigned int i = 0 ; i < list.size() ; ++i)
			m_parts.push_back(new maildirPart(parent, number, *list[i]));
	}

	~maildirStructure()
	{
		free_container(m_parts);
	}


	const part& operator[](const int x) const
	{
		return (*m_parts[x - 1]);
	}

	part& operator[](const int x)
	{
		return (*m_parts[x - 1]);
	}

	const int getCount() const
	{
		return (m_parts.size());
	}


	static maildirStructure* emptyStructure()
	{
		return (&m_emptyStructure);
	}

private:

	static maildirStructure m_emptyStructure;

	std::vector <maildirPart*> m_parts;
};


maildirStructure maildirStructure::m_emptyStructure;



maildirPart::maildirPart(maildirPart* parent, const int number, const bodyPart& part)
	: m_parent(parent), m_header(NULL), m_number(number)
{
	if (part.getBody()->getPartList().size() == 0)
		m_structure = NULL;
	else
		m_structure = new maildirStructure(this, part.getBody()->getPartList());

	m_headerParsedOffset = part.getHeader()->getParsedOffset();
	m_headerParsedLength = part.getHeader()->getParsedLength();

	m_bodyParsedOffset = part.getBody()->getParsedOffset();
	m_bodyParsedLength = part.getBody()->getParsedLength();

	m_size = part.getBody()->getContents().getLength();

	m_mediaType = part.getBody()->getContentType();
}


maildirPart::~maildirPart()
{
	delete (m_structure);
	delete (m_header);
}


const structure& maildirPart::getStructure() const
{
	if (m_structure != NULL)
		return (*m_structure);
	else
		return (*maildirStructure::emptyStructure());
}


structure& maildirPart::getStructure()
{
	if (m_structure != NULL)
		return (*m_structure);
	else
		return (*maildirStructure::emptyStructure());
}



//
// maildirMessage
//

maildirMessage::maildirMessage(maildirFolder* folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_header(NULL), m_structure(NULL)
{
	m_folder->registerMessage(this);
}


maildirMessage::~maildirMessage()
{
	if (m_folder)
		m_folder->unregisterMessage(this);

	delete (m_header);
	delete (m_structure);
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
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return (*m_structure);
}


structure& maildirMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return (*m_structure);
}


const header& maildirMessage::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (*m_header);
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
	extractImpl(os, progress, 0, m_size, start, length);
}


void maildirMessage::extractPart(const part& p, utility::outputStream& os,
	progressionListener* progress, const int start, const int length) const
{
	const maildirPart& mp = dynamic_cast <const maildirPart&>(p);

	extractImpl(os, progress, mp.getBodyParsedOffset(), mp.getBodyParsedLength(), start, length);
}


void maildirMessage::extractImpl(utility::outputStream& os, progressionListener* progress,
	const int start, const int length, const int partialStart, const int partialLength) const
{
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = m_folder->getMessageFSPath(m_num);
	utility::auto_ptr <utility::file> file = fsf->create(path);

	utility::auto_ptr <utility::fileReader> reader = file->getFileReader();
	utility::auto_ptr <utility::inputStream> is = reader->getInputStream();

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
}


void maildirMessage::fetchPartHeader(part& p)
{
	maildirPart& mp = dynamic_cast <maildirPart&>(p);

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = m_folder->getMessageFSPath(m_num);
	utility::auto_ptr <utility::file> file = fsf->create(path);

	utility::auto_ptr <utility::fileReader> reader = file->getFileReader();
	utility::auto_ptr <utility::inputStream> is = reader->getInputStream();

	is->skip(mp.getHeaderParsedOffset());

	utility::stream::value_type buffer[1024];
	utility::stream::size_type remaining = mp.getHeaderParsedLength();

	string contents;
	contents.reserve(remaining);

	while (!is->eof() && remaining > 0)
	{
		const utility::stream::size_type read =
			is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;

		contents.append(buffer, read);
	}

	mp.getOrCreateHeader().parse(contents);
}


void maildirMessage::fetch(maildirFolder* folder, const int options)
{
	if (m_folder != folder)
		throw exceptions::folder_not_found();

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	utility::auto_ptr <utility::file> file = fsf->create(path);

	if (options & folder::FETCH_FLAGS)
		m_flags = maildirUtils::extractFlags(path.getLastComponent());

	if (options & folder::FETCH_SIZE)
		m_size = file->getLength();

	if (options & folder::FETCH_UID)
		m_uid = maildirUtils::extractId(path.getLastComponent()).getBuffer();

	if (options & (folder::FETCH_ENVELOPE | folder::FETCH_CONTENT_INFO |
	               folder::FETCH_FULL_HEADER | folder::FETCH_STRUCTURE))
	{
		string contents;

		utility::auto_ptr <utility::fileReader> reader = file->getFileReader();
		utility::auto_ptr <utility::inputStream> is = reader->getInputStream();

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
			if (m_structure)
				delete (m_structure);

			m_structure = new maildirStructure(NULL, msg);
		}

		// Extract some header fields or whole header
		if (options & (folder::FETCH_ENVELOPE |
		               folder::FETCH_CONTENT_INFO |
		               folder::FETCH_FULL_HEADER))
		{
			getOrCreateHeader().copyFrom(*(msg.getHeader()));
		}
	}
}


header& maildirMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (*m_header);
	else
		return (*(m_header = new header()));
}


} // messaging
} // vmime
