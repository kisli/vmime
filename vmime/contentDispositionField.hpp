//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED


#include "vmime/parameterizedHeaderField.hpp"
#include "vmime/genericField.hpp"

#include "vmime/contentDisposition.hpp"
#include "vmime/dateTime.hpp"


namespace vmime
{


class contentDispositionField : public parameterizedHeaderField, public genericField <contentDisposition>
{
	friend class headerFieldFactory::registerer <contentDispositionField>;

protected:

	contentDispositionField();
	contentDispositionField(contentDispositionField&);

public:

	const datetime& getCreationDate() const;
	void setCreationDate(const datetime& creationDate);

	const datetime& getModificationDate() const;
	void setModificationDate(const datetime& modificationDate);

	const datetime& getReadDate() const;
	void setReadDate(const datetime& readDate);

	const string getFilename() const;
	void setFilename(const string& filename);

	const string getSize() const;
	void setSize(const string& size);
};


} // vmime


#endif // VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
