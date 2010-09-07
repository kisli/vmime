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

#ifndef VMIME_ATTACHMENTHELPER_HPP_INCLUDED
#define VMIME_ATTACHMENTHELPER_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/attachment.hpp"
#include "vmime/message.hpp"


namespace vmime
{


/** Retrieve attachment information from message parts.
  */
class attachmentHelper
{
public:

	/** Options for use with the following functions:
	  *     findAttachmentsInMessage,
	  *     getBodyPartAttachment,
	  * and isBodyPartAnAttachment.
	  */
	enum FindOptions
	{
		INLINE_OBJECTS = (1 << 0)        /**< Recognize and return inline objects. The aim is to
		                                      consider MHTML objects (parts with a "Content-Id" or
		                                      a "Content-Location", such as inline images) as attachments. */
	};

	/** Test whether a body part is an attachment.
	  *
	  * @param part message part to test
	  * @param options search options (see FindOptions)
	  * @return true if the part is an attachment, false otherwise
	  */
	static bool isBodyPartAnAttachment(ref <const bodyPart> part, const unsigned int options = 0);

	/** Return attachment information in the specified body part.
	  * If the specified body part does not contain attachment
	  * information (ie. is not an attachment), NULL is returned.
	  *
	  * @param part message part in which to search
	  * @param options search options (see FindOptions)
	  * @return attachment found in the part, or NULL
	  */
	static ref <const attachment>
		getBodyPartAttachment(ref <const bodyPart> part, const unsigned int options = 0);

	/** Find all attachments contained in the specified part
	  * and all its children parts.
	  * This is simply a recursive call to getBodyPartAttachment().
	  *
	  * @param part part in which to search
	  * @param options search options (see FindOptions)
	  * @return a list of attachments found
	  */
	static const std::vector <ref <const attachment> >
		findAttachmentsInBodyPart(ref <const bodyPart> part, const unsigned int options = 0);

	/** Find all attachments contained in the specified message.
	  * This is simply a recursive call to getBodyPartAttachment().
	  *
	  * @param msg message in which to search
	  * @param options search options (see FindOptions)
	  * @return a list of attachments found
	  */
	static const std::vector <ref <const attachment> >
		findAttachmentsInMessage(ref <const message> msg, const unsigned int options = 0);

	/** Add an attachment to the specified message.
	  *
	  * @param msg message into which to add the attachment
	  * @param att attachment to add
	  */
	static void addAttachment(ref <message> msg, ref <attachment> att);

	/** Add a message attachment to the specified message.
	  *
	  * @param msg message into which to add the attachment
	  * @param amsg message to attach
	  */
	static void addAttachment(ref <message> msg, ref <message> amsg);

protected:

	static ref <bodyPart> findBodyPart
		(ref <bodyPart> part, const mediaType& type);
};


} // vmime


#endif // VMIME_ATTACHMENTHELPER_HPP_INCLUDED

