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

#ifndef VMIME_FILEATTACHMENT_HPP_INCLUDED
#define VMIME_FILEATTACHMENT_HPP_INCLUDED


#include "defaultAttachment.hpp"


namespace vmime
{


class fileAttachment : public defaultAttachment
{
public:

	fileAttachment(const string& filename, const mediaType& type, const text& desc = NULL_TEXT);
	fileAttachment(const string& filename, const mediaType& type, const encoding& enc, const text& desc = NULL_TEXT);

	class fileInfo
	{
	public:

		fileInfo();
		~fileInfo();

		const bool hasFilename() const;
		const string& getFilename() const;
		void setFilename(const string& name);

		const bool hasCreationDate() const;
		const datetime& getCreationDate() const;
		void setCreationDate(const datetime& date);

		const bool hasModificationDate() const;
		const datetime& getModificationDate() const;
		void setModificationDate(const datetime& date);

		const bool hasReadDate() const;
		const datetime& getReadDate() const;
		void setReadDate(const datetime& date);

		const bool hasSize() const;
		const unsigned int getSize() const;
		void setSize(const unsigned int& size);

	protected:

		string* m_filename;
		unsigned int* m_size;
		datetime* m_creationDate;
		datetime* m_modifDate;
		datetime* m_readDate;
	};

	const fileInfo& getFileInfo() const;
	fileInfo& getFileInfo();

private:

	void setData(const string& filename);

	fileInfo m_fileInfo;

	void generatePart(bodyPart& part) const;
};


} // vmime


#endif // VMIME_FILEATTACHMENT_HPP_INCLUDED
