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

#ifndef VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED


#include "vmime/parameterizedHeaderField.hpp"

#include "vmime/contentDisposition.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/word.hpp"


namespace vmime
{


class contentDispositionField : public parameterizedHeaderField
{
	friend class vmime::creator;  // create ref

protected:

	contentDispositionField();
	contentDispositionField(contentDispositionField&);

public:

	/** Return the value of the "creation-date" parameter.
	  *
	  * @return value of the "creation-date" parameter
	  * @throw exceptions::no_such_parameter if the parameter does not exist
	  */
	const datetime getCreationDate() const;

	/** Set the value of the "creation-date" parameter.
	  *
	  * @param creationDate new value for the "creation-date" parameter
	  */
	void setCreationDate(const datetime& creationDate);

	/** Return the value of the "modification-date" parameter.
	  *
	  * @return value of the "modification-date" parameter
	  * @throw exceptions::no_such_parameter if the parameter does not exist
	  */
	const datetime getModificationDate() const;

	/** Set the value of the "modification-date" parameter.
	  *
	  * @param modificationDate new value for the "modification-date" parameter
	  */
	void setModificationDate(const datetime& modificationDate);

	/** Return the value of the "read-date" parameter.
	  *
	  * @return value of the "read-date" parameter
	  * @throw exceptions::no_such_parameter if the parameter does not exist
	  */
	const datetime getReadDate() const;

	/** Set the value of the "read-date" parameter.
	  *
	  * @param readDate new value for the "read-date" parameter
	  */
	void setReadDate(const datetime& readDate);

	/** Return the value of the "filename" parameter.
	  *
	  * @return value of the "filename" parameter
	  * @throw exceptions::no_such_parameter if the parameter does not exist
	  */
	const word getFilename() const;

	/** Set the value of the "filename" parameter.
	  *
	  * @param filename new value for the "filename" parameter
	  */
	void setFilename(const word& filename);

	/** Return the value of the "size" parameter.
	  *
	  * @return value of the "size" parameter
	  * @throw exceptions::no_such_parameter if the parameter does not exist
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
