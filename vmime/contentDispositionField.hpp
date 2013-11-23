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

#ifndef VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED


#include "vmime/parameterizedHeaderField.hpp"

#include "vmime/contentDisposition.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/word.hpp"


namespace vmime
{


/** Describes presentation information, as per RFC-2183.
  */

class VMIME_EXPORT contentDispositionField : public parameterizedHeaderField
{
	friend class headerFieldFactory;

protected:

	contentDispositionField();
	contentDispositionField(contentDispositionField&);

public:

	/** Test whether the "creation-date" parameter is set.
	  *
	  * @return true if the "creation-date" parameter is set, or false otherwise
	  */
	bool hasCreationDate() const;

	/** Return the value of the "creation-date" parameter.
	  *
	  * @return value of the "creation-date" parameter
	  */
	const datetime getCreationDate() const;

	/** Set the value of the "creation-date" parameter.
	  *
	  * @param creationDate new value for the "creation-date" parameter
	  */
	void setCreationDate(const datetime& creationDate);

	/** Test whether the "modification-date" parameter is set.
	  *
	  * @return true if the "modification-date" parameter is set, or false otherwise
	  */
	bool hasModificationDate() const;

	/** Return the value of the "modification-date" parameter.
	  *
	  * @return value of the "modification-date" parameter
	  */
	const datetime getModificationDate() const;

	/** Set the value of the "modification-date" parameter.
	  *
	  * @param modificationDate new value for the "modification-date" parameter
	  */
	void setModificationDate(const datetime& modificationDate);

	/** Test whether the "read-date" parameter is set.
	  *
	  * @return true if the "read-date" parameter is set, or false otherwise
	  */
	bool hasReadDate() const;

	/** Return the value of the "read-date" parameter.
	  *
	  * @return value of the "read-date" parameter
	  */
	const datetime getReadDate() const;

	/** Set the value of the "read-date" parameter.
	  *
	  * @param readDate new value for the "read-date" parameter
	  */
	void setReadDate(const datetime& readDate);

	/** Test whether the "filename" parameter is set.
	  *
	  * @return true if the "filename" parameter is set, or false otherwise
	  */
	bool hasFilename() const;

	/** Return the value of the "filename" parameter.
	  *
	  * @return value of the "filename" parameter
	  */
	const word getFilename() const;

	/** Set the value of the "filename" parameter.
	  *
	  * @param filename new value for the "filename" parameter
	  */
	void setFilename(const word& filename);

	/** Test whether the "size" parameter is set.
	  *
	  * @return true if the "size" parameter is set, or false otherwise
	  */
	bool hasSize() const;

	/** Return the value of the "size" parameter.
	  *
	  * @return value of the "size" parameter
	  */
	const string getSize() const;

	/** Set the value of the "size" parameter.
	  *
	  * @param size new value for the "size" parameter
	  */
	void setSize(const string& size);
};


} // vmime


#endif // VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
