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

#include <fstream>
#include <sstream>

#include "fileAttachment.hpp"
#include "exception.hpp"


namespace vmime
{


fileAttachment::fileAttachment(const string& filename, const mediaType& type, const text& desc)
{
	m_type = type;
	m_desc = desc;

	setData(filename);

	m_encoding = encoding::decide(m_data);
}


fileAttachment::fileAttachment(const string& filename, const mediaType& type,
	const class encoding& enc, const text& desc)
{
	m_type = type;
	m_desc = desc;

	setData(filename);

	m_encoding = enc;
}


void fileAttachment::setData(const string& filename)
{
	std::ifstream* file = new std::ifstream();
	file->open(filename.c_str(), std::ios::in | std::ios::binary);

	if (!*file)
	{
		delete (file);
		throw exceptions::open_file_error();
	}

	m_data.set(new utility::inputStreamPointerAdapter(file, true), 0, true);
}


void fileAttachment::generatePart(bodyPart& part) const
{
	defaultAttachment::generatePart(part);

	contentDispositionField& cdf = part.header().fields.ContentDisposition();

	if (m_fileInfo.hasSize()) cdf.size() = toString(m_fileInfo.getSize());
	if (m_fileInfo.hasFilename()) cdf.filename() = m_fileInfo.getFilename();
	if (m_fileInfo.hasCreationDate()) cdf.creationDate() = m_fileInfo.getCreationDate();
	if (m_fileInfo.hasModificationDate()) cdf.modificationDate() = m_fileInfo.getModificationDate();
	if (m_fileInfo.hasReadDate()) cdf.readDate() = m_fileInfo.getReadDate();
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

const bool fileAttachment::fileInfo::hasFilename() const { return (m_filename != NULL); }
const string& fileAttachment::fileInfo::getFilename() const { return (*m_filename); }
void fileAttachment::fileInfo::setFilename(const string& name) { if (m_filename) { *m_filename = name; } else { m_filename = new string(name); } }

const bool fileAttachment::fileInfo::hasCreationDate() const { return (m_creationDate != NULL); }
const datetime& fileAttachment::fileInfo::getCreationDate() const { return (*m_creationDate); }
void fileAttachment::fileInfo::setCreationDate(const datetime& date) { if (m_creationDate) { *m_creationDate = date; } else { m_creationDate = new datetime(date); } }

const bool fileAttachment::fileInfo::hasModificationDate() const { return (m_modifDate != NULL); }
const datetime& fileAttachment::fileInfo::getModificationDate() const { return (*m_modifDate); }
void fileAttachment::fileInfo::setModificationDate(const datetime& date) { if (m_modifDate) { *m_modifDate = date; } else { m_modifDate = new datetime(date); } }

const bool fileAttachment::fileInfo::hasReadDate() const { return (m_readDate != NULL); }
const datetime& fileAttachment::fileInfo::getReadDate() const { return (*m_readDate); }
void fileAttachment::fileInfo::setReadDate(const datetime& date) { if (m_readDate) { *m_readDate = date; } else { m_readDate = new datetime(date); } }

const bool fileAttachment::fileInfo::hasSize() const { return (m_size != NULL); }
const unsigned int fileAttachment::fileInfo::getSize() const { return (*m_size); }
void fileAttachment::fileInfo::setSize(const unsigned int& size) { if (m_size) { *m_size = size; } else { m_size = new unsigned int(size); } }


} // vmime
