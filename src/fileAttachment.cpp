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

#include <fstream>
#include <sstream>

#include "vmime/fileAttachment.hpp"
#include "vmime/exception.hpp"

#include "vmime/streamContentHandler.hpp"
#include "vmime/utility/inputStreamPointerAdapter.hpp"

#include "vmime/contentDispositionField.hpp"

#include "vmime/platform.hpp"
#include "vmime/utility/file.hpp"


namespace vmime
{


fileAttachment::fileAttachment(const string& filepath, const mediaType& type)
{
	m_type = type;

	setData(filepath);

	m_encoding = encoding::decide(m_data);
}


fileAttachment::fileAttachment(const string& filepath, const mediaType& type, const text& desc)
{
	m_type = type;
	m_desc = desc;

	setData(filepath);

	m_encoding = encoding::decide(m_data);
}


fileAttachment::fileAttachment(const string& filepath, const mediaType& type,
	const text& desc, const encoding& enc)
{
	m_type = type;
	m_desc = desc;

	setData(filepath);

	m_encoding = enc;
}


fileAttachment::fileAttachment(ref <utility::inputStream> is, const word& filename, const mediaType& type)
{
	if (!filename.isEmpty())
		m_fileInfo.setFilename(filename);

	m_type = type;

	setData(is);

	m_encoding = encoding::decide(m_data);
}


fileAttachment::fileAttachment(ref <utility::inputStream> is, const word& filename,
	const mediaType& type, const text& desc)
{
	if (!filename.isEmpty())
		m_fileInfo.setFilename(filename);

	m_type = type;
	m_desc = desc;

	setData(is);

	m_encoding = encoding::decide(m_data);
}


fileAttachment::fileAttachment(ref <utility::inputStream> is, const word& filename,
	const mediaType& type, const text& desc, const encoding& enc)
{
	if (!filename.isEmpty())
		m_fileInfo.setFilename(filename);

	m_type = type;
	m_desc = desc;
	m_encoding = enc;

	setData(is);
}


void fileAttachment::setData(const string& filepath)
{
	std::ifstream* file = new std::ifstream();
	file->open(filepath.c_str(), std::ios::in | std::ios::binary);

	if (!*file)
	{
		delete file;
		throw exceptions::open_file_error();
	}

	ref <utility::inputStream> is = vmime::create <utility::inputStreamPointerAdapter>(file, true);

	setData(is);

	utility::file::path path = platform::getHandler()->getFileSystemFactory()->stringToPath(filepath);
	m_fileInfo.setFilename(path.getLastComponent());
}


void fileAttachment::setData(ref <utility::inputStream> is)
{
	m_data = vmime::create <streamContentHandler>(is, 0);
}


void fileAttachment::generatePart(ref <bodyPart> part) const
{
	defaultAttachment::generatePart(part);

	ref <contentDispositionField> cdf = part->getHeader()->ContentDisposition().
		dynamicCast <contentDispositionField>();

	if (m_fileInfo.hasSize()) cdf->setSize(utility::stringUtils::toString(m_fileInfo.getSize()));
	if (m_fileInfo.hasFilename() && !m_fileInfo.getFilename().isEmpty()) cdf->setFilename(m_fileInfo.getFilename());
	if (m_fileInfo.hasCreationDate()) cdf->setCreationDate(m_fileInfo.getCreationDate());
	if (m_fileInfo.hasModificationDate()) cdf->setModificationDate(m_fileInfo.getModificationDate());
	if (m_fileInfo.hasReadDate()) cdf->setReadDate(m_fileInfo.getReadDate());
}


const fileAttachment::fileInfo& fileAttachment::getFileInfo() const
{
	return m_fileInfo;
}


fileAttachment::fileInfo& fileAttachment::getFileInfo()
{
	return m_fileInfo;
}



//
// fileAttachment::fileInfo
//

fileAttachment::fileInfo::fileInfo()
	: m_filename(NULL), m_size(NULL), m_creationDate(NULL), m_modifDate(NULL), m_readDate(NULL)
{
}


fileAttachment::fileInfo::~fileInfo()
{
	delete (m_filename);
	delete (m_size);
	delete (m_creationDate);
	delete (m_modifDate);
	delete (m_readDate);
}

bool fileAttachment::fileInfo::hasFilename() const { return (m_filename != NULL); }
const word& fileAttachment::fileInfo::getFilename() const { return (*m_filename); }
void fileAttachment::fileInfo::setFilename(const string& name) { if (m_filename) { *m_filename = name; } else { m_filename = new word(name); } }
void fileAttachment::fileInfo::setFilename(const word& name) { if (m_filename) { *m_filename = name; } else { m_filename = new word(name); } }

bool fileAttachment::fileInfo::hasCreationDate() const { return (m_creationDate != NULL); }
const datetime& fileAttachment::fileInfo::getCreationDate() const { return (*m_creationDate); }
void fileAttachment::fileInfo::setCreationDate(const datetime& date) { if (m_creationDate) { *m_creationDate = date; } else { m_creationDate = new datetime(date); } }

bool fileAttachment::fileInfo::hasModificationDate() const { return (m_modifDate != NULL); }
const datetime& fileAttachment::fileInfo::getModificationDate() const { return (*m_modifDate); }
void fileAttachment::fileInfo::setModificationDate(const datetime& date) { if (m_modifDate) { *m_modifDate = date; } else { m_modifDate = new datetime(date); } }

bool fileAttachment::fileInfo::hasReadDate() const { return (m_readDate != NULL); }
const datetime& fileAttachment::fileInfo::getReadDate() const { return (*m_readDate); }
void fileAttachment::fileInfo::setReadDate(const datetime& date) { if (m_readDate) { *m_readDate = date; } else { m_readDate = new datetime(date); } }

bool fileAttachment::fileInfo::hasSize() const { return (m_size != NULL); }
unsigned int fileAttachment::fileInfo::getSize() const { return (*m_size); }
void fileAttachment::fileInfo::setSize(const unsigned int& size) { if (m_size) { *m_size = size; } else { m_size = new unsigned int(size); } }


} // vmime
