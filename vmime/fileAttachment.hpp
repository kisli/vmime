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

#ifndef VMIME_FILEATTACHMENT_HPP_INCLUDED
#define VMIME_FILEATTACHMENT_HPP_INCLUDED


#include "vmime/defaultAttachment.hpp"

#include "vmime/dateTime.hpp"


namespace vmime
{


/** Attachment of type 'file'.
  */

class fileAttachment : public defaultAttachment
{
public:

	fileAttachment(const string& filepath, const mediaType& type);
	fileAttachment(const string& filepath, const mediaType& type, const text& desc);
	fileAttachment(const string& filepath, const mediaType& type, const text& desc, const encoding& enc);

	fileAttachment(ref <utility::inputStream> is, const word& filename, const mediaType& type);
	fileAttachment(ref <utility::inputStream> is, const word& filename, const mediaType& type, const text& desc);
	fileAttachment(ref <utility::inputStream> is, const word& filename, const mediaType& type, const text& desc, const encoding& enc);

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
		bool hasFilename() const;

		/** Return the value of the 'filename' property.
		  *
		  * @return file name
		  */
		const word& getFilename() const;

		/** Set the value of the 'filename' property.
		  *
		  * @param name file name
		  */
		void setFilename(const string& name);

		/** Set the value of the 'filename' property.
		  *
		  * @param name file name
		  */
		void setFilename(const word& name);

		/** Check whether the 'creation-date' property is present.
		  *
		  * @return true if the 'creation-date' property is set,
		  * false otherwise
		  */
		bool hasCreationDate() const;

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
		bool hasModificationDate() const;

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
		bool hasReadDate() const;

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
		bool hasSize() const;

		/** Return the value of the 'size' property.
		  *
		  * @return file size
		  */
		unsigned int getSize() const;

		/** Set the value of the 'size' property.
		  *
		  * @param size file size
		  */
		void setSize(const unsigned int& size);

	private:

		word* m_filename;
		unsigned int* m_size;
		datetime* m_creationDate;
		datetime* m_modifDate;
		datetime* m_readDate;
	};

	const fileInfo& getFileInfo() const;
	fileInfo& getFileInfo();

private:

	void setData(const string& filepath);
	void setData(ref <utility::inputStream> is);

	fileInfo m_fileInfo;

	void generatePart(ref <bodyPart> part) const;
};


} // vmime


#endif // VMIME_FILEATTACHMENT_HPP_INCLUDED
