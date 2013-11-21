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

#ifndef VMIME_GENERATEDMESSAGEATTACHMENT_HPP_INCLUDED
#define VMIME_GENERATEDMESSAGEATTACHMENT_HPP_INCLUDED


#ifndef VMIME_BUILDING_DOC  // implementation detail


#include "vmime/messageAttachment.hpp"
#include "vmime/bodyPartAttachment.hpp"


namespace vmime
{


/** A message attachment that can be extracted from a message.
  */
class VMIME_EXPORT generatedMessageAttachment : public messageAttachment
{
public:

	generatedMessageAttachment(shared_ptr <const bodyPart> part);

	const mediaType getType() const;
	const text getDescription() const;
	const word getName() const;

	const shared_ptr <const contentHandler> getData() const;

	const encoding getEncoding() const;

	shared_ptr <const object> getPart() const;

	shared_ptr <const header> getHeader() const;

	shared_ptr <message> getMessage() const;

protected:

	void generateIn(shared_ptr <bodyPart> parent) const;

private:

	shared_ptr <bodyPartAttachment> m_bpa;
	mutable shared_ptr <message> m_msg;
};


} // vmime


#endif // !VMIME_BUILDING_DOC


#endif // VMIME_GENERATEDMESSAGEATTACHMENT_HPP_INCLUDED

