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

#include "contentDispositionField.hpp"
#include "exception.hpp"

#include "standardParams.hpp"


namespace vmime
{


contentDispositionField::contentDispositionField()
{
}


contentDispositionField::contentDispositionField(contentDispositionField&)
	: headerField(), parameterizedHeaderField(), genericField <contentDisposition>()
{
}


const datetime& contentDispositionField::getCreationDate() const
{
	return (dynamic_cast <const dateParameter&>(*findParameter("creation-date")).getValue());
}


void contentDispositionField::setCreationDate(const datetime& creationDate)
{
	dynamic_cast <dateParameter&>(*getParameter("creation-date")).setValue(creationDate);
}


const datetime& contentDispositionField::getModificationDate() const
{
	return (dynamic_cast <const dateParameter&>(*findParameter("modification-date")).getValue());
}


void contentDispositionField::setModificationDate(const datetime& modificationDate)
{
	dynamic_cast <dateParameter&>(*getParameter("modification-date")).setValue(modificationDate);
}


const datetime& contentDispositionField::getReadDate() const
{
	return (dynamic_cast <const dateParameter&>(*findParameter("read-date")).getValue());
}


void contentDispositionField::setReadDate(const datetime& readDate)
{
	dynamic_cast <dateParameter&>(*getParameter("read-date")).setValue(readDate);
}


const string contentDispositionField::getFilename() const
{
	return (dynamic_cast <const defaultParameter&>(*findParameter("filename")).getValue());
}


void contentDispositionField::setFilename(const string& filename)
{
	dynamic_cast <defaultParameter&>(*getParameter("filename")).setValue(filename);
}


const string contentDispositionField::getSize() const
{
	return (dynamic_cast <const defaultParameter&>(*findParameter("size")).getValue());
}


void contentDispositionField::setSize(const string& size)
{
	dynamic_cast <defaultParameter&>(*getParameter("size")).setValue(size);
}


} // vmime
