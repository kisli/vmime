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

#include "vmime/contentDispositionField.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


contentDispositionField::contentDispositionField()
{
}


contentDispositionField::contentDispositionField(contentDispositionField&)
	: parameterizedHeaderField()
{
}


bool contentDispositionField::hasCreationDate() const
{
	return hasParameter("creation-date");
}


const datetime contentDispositionField::getCreationDate() const
{
	shared_ptr <parameter> param = findParameter("creation-date");

	if (param)
		return param->getValueAs <datetime>();
	else
		return datetime::now();
}


void contentDispositionField::setCreationDate(const datetime& creationDate)
{
	getParameter("creation-date")->setValue(creationDate);
}


bool contentDispositionField::hasModificationDate() const
{
	return hasParameter("modification-date");
}


const datetime contentDispositionField::getModificationDate() const
{
	shared_ptr <parameter> param = findParameter("modification-date");

	if (param)
		return param->getValueAs <datetime>();
	else
		return datetime::now();
}


void contentDispositionField::setModificationDate(const datetime& modificationDate)
{
	getParameter("modification-date")->setValue(modificationDate);
}


bool contentDispositionField::hasReadDate() const
{
	return hasParameter("read-date");
}


const datetime contentDispositionField::getReadDate() const
{
	shared_ptr <parameter> param = findParameter("read-date");

	if (param)
		return param->getValueAs <datetime>();
	else
		return datetime::now();
}


void contentDispositionField::setReadDate(const datetime& readDate)
{
	getParameter("read-date")->setValue(readDate);
}


bool contentDispositionField::hasFilename() const
{
	return hasParameter("filename");
}


const word contentDispositionField::getFilename() const
{
	shared_ptr <parameter> param = findParameter("filename");

	if (param)
		return param->getValue();
	else
		return word();
}


void contentDispositionField::setFilename(const word& filename)
{
	getParameter("filename")->setValue(filename);
}


bool contentDispositionField::hasSize() const
{
	return hasParameter("size");
}


const string contentDispositionField::getSize() const
{
	shared_ptr <parameter> param = findParameter("size");

	if (param)
		return param->getValue().getBuffer();
	else
		return "";
}


void contentDispositionField::setSize(const string& size)
{
	getParameter("size")->setValue(word(size, vmime::charsets::US_ASCII));
}


} // vmime
