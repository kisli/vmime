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

#include "vmime/fileContentHandler.hpp"


namespace vmime
{


fileContentHandler::fileContentHandler()
	: streamContentHandler()
{
}


fileContentHandler::fileContentHandler
	(shared_ptr <utility::file> file, const vmime::encoding& enc)
{
	setData(file, enc);
}


fileContentHandler::~fileContentHandler()
{
}


fileContentHandler::fileContentHandler(const fileContentHandler& cts)
	: streamContentHandler()
{
	setData(cts.m_file, cts.m_encoding);
}


fileContentHandler& fileContentHandler::operator=(const fileContentHandler& cts)
{
	setData(cts.m_file, cts.m_encoding);

	return *this;
}


shared_ptr <contentHandler> fileContentHandler::clone() const
{
	return make_shared <fileContentHandler>(*this);
}


void fileContentHandler::setData
	(shared_ptr <utility::file> file, const vmime::encoding& enc)
{
	m_file = file;
	m_encoding = enc;

	streamContentHandler::setData
		(file->getFileReader()->getInputStream(), file->getLength(), enc);
}


} // vmime
