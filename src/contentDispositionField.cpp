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

#include "vmime/contentDispositionField.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


contentDispositionField::contentDispositionField()
{
}


contentDispositionField::contentDispositionField(contentDispositionField&)
	: headerField(), parameterizedHeaderField()
{
}


const datetime contentDispositionField::getCreationDate() const
{
	return findParameter("creation-date")->getValueAs <datetime>();
}


void contentDispositionField::setCreationDate(const datetime& creationDate)
{
	getParameter("creation-date")->setValue(creationDate);
}


const datetime contentDispositionField::getModificationDate() const
{
	return findParameter("modification-date")->getValueAs <datetime>();
}


void contentDispositionField::setModificationDate(const datetime& modificationDate)
{
	getParameter("modification-date")->setValue(modificationDate);
}


const datetime contentDispositionField::getReadDate() const
{
	return findParameter("read-date")->getValueAs <datetime>();
}


void contentDispositionField::setReadDate(const datetime& readDate)
{
	getParameter("read-date")->setValue(readDate);
}


const word contentDispositionField::getFilename() const
{
	return findParameter("filename")->getValue();
}


void contentDispositionField::setFilename(const word& filename)
{
	getParameter("filename")->setValue(filename);
}


const string contentDispositionField::getSize() const
{
	return findParameter("size")->getValue().getBuffer();
}


void contentDispositionField::setSize(const string& size)
{
	getParameter("size")->setValue(word(size, vmime::charsets::US_ASCII));
}


} // vmime
