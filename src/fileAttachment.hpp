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


/** Attachment of type 'file'.
  */

class fileAttachment : public defaultAttachment
{
public:

	fileAttachment(const string& filename, const mediaType& type, const text& desc = NULL_TEXT);
	fileAttachment(const string& filename, const mediaType& type, const encoding& enc, const text& desc = NULL_TEXT);

	/** Stores information about a file attachment.
	  */
	class fileInfo
	{
	public:

		fileInfo();
		~fileInfo();

		/** Check whether the 'filename' property is present.
		  *
		  * @return true if the 'filename' property is set,
		  * false otherwise
		  */
		const bool hasFilename() const;

		/** Return the value of the 'filename' property.
		  *
		  * @return file name
		  */
		const string& getFilename() const;

		/** Set the value of the 'filename' property.
		  *
		  * @param name file name
		  */
		void setFilename(const string& name);

		/** Check whether the 'creation-date' property is present.
		  *
		  * @return true if the 'creation-date' property is set,
		  * false otherwise
		  */
		const bool hasCreationDate() const;

		/** Return the value of the 'creation-date' property.
		  *
		  * @return file creation time
		  */
		const datetime& getCreationDate() const;

		/** Set the value of the 'creation-date' property.
		  *
		  * @param date file creation time
		  */
		void setCreationDate(const datetime& date);

		/** Check whether the 'modification-date' property is present.
		  *
		  * @return true if the 'modification-date' property is set,
		  * false otherwise
		  */
		const bool hasModificationDate() const;

		/** Return the value of the 'modification-date' property.
		  *
		  * @return file modification time
		  */
		const datetime& getModificationDate() const;

		/** Set the value of the 'modification-date' property.
		  *
		  * @param date file modification time
		  */
		void setModificationDate(const datetime& date);

		/** Check whether the 'read-date' property is set.
		  *
		  * @return true if the 'read-date' property is set,
		  * false otherwise
		  */
		const bool hasReadDate() const;

		/** Return the value of the 'read-date' property.
		  *
		  * @return file access time
		  */
		const datetime& getReadDate() const;

		/** Set the value of the 'read-date' property.
		  *
		  * @param date file access time
		  */
		void setReadDate(const datetime& date);

		/** Check whether the value of the 'size' property is set.
		  *
		  * @return true if the 'size' property is set,
		  * false otherwise
		  */
		const bool hasSize() const;

		/** Return the value of the 'size' property.
		  *
		  * @return file size
		  */
		const unsigned int getSize() const;

		/** Set the value of the 'size' property.
		  *
		  * @param size file size
		  */
		void setSize(const unsigned int& size);

	private:

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
