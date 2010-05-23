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

#include "vmime/emptyContentHandler.hpp"


namespace vmime
{


emptyContentHandler::emptyContentHandler()
{
}


ref <contentHandler> emptyContentHandler::clone() const
{
	return vmime::create <emptyContentHandler>();
}


void emptyContentHandler::generate(utility::outputStream& /* os */, const vmime::encoding& /* enc */,
	const string::size_type /* maxLineLength */) const
{
	// Nothing to do.
}


void emptyContentHandler::extract(utility::outputStream& /* os */,
	utility::progressListener* progress) const
{
	if (progress)
		progress->start(0);

	// Nothing to do.

	if (progress)
		progress->stop(0);
}


void emptyContentHandler::extractRaw(utility::outputStream& /* os */,
	utility::progressListener* progress) const
{
	if (progress)
		progress->start(0);

	// Nothing to do.

	if (progress)
		progress->stop(0);
}


string::size_type emptyContentHandler::getLength() const
{
	return (0);
}


bool emptyContentHandler::isEmpty() const
{
	return (true);
}


bool emptyContentHandler::isEncoded() const
{
	return (false);
}


const vmime::encoding& emptyContentHandler::getEncoding() const
{
	return (NO_ENCODING);
}


bool emptyContentHandler::isBuffered() const
{
	return true;
}


} // vmime
