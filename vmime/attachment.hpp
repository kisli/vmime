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

#ifndef VMIME_ATTACHMENT_HPP_INCLUDED
#define VMIME_ATTACHMENT_HPP_INCLUDED


#include "vmime/base.hpp"

#include "vmime/bodyPart.hpp"
#include "vmime/mediaType.hpp"
#include "vmime/text.hpp"
#include "vmime/contentHandler.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


/** Base class for all types of attachment.
  */

class attachment : public object
{
	friend class messageBuilder;
	friend class messageParser;
	friend class attachmentHelper;

protected:

	attachment() { }

public:

	virtual ~attachment() { }

	/** Return the media type of this attachment.
	  *
	  * @return content type of the attachment
	  */
	virtual const mediaType getType() const = 0;

	/** Return the description of this attachment.
	  *
	  * @return attachment description, or an empty text
	  * if no description is available
	  */
	virtual const text getDescription() const = 0;

	/** Return the (file) name of this attachment.
	  *
	  * @return attachment name, or an empty word if no
	  * name is available
	  */
	virtual const word getName() const = 0;

	/** Return the data contained in this attachment.
	  *
	  * @return attachment data
	  */
	virtual const ref <const contentHandler> getData() const = 0;

	/** Return the encoding used for this attachment.
	  *
	  * @return attachment data encoding
	  */
	virtual const encoding getEncoding() const = 0;

	/** Return the part in which the attachment has been found.
	  * This can be a vmime::bodyPart or a vmime::net::part object.
	  *
	  * @return attachment part or NULL if the attachment is not
	  * attached to a part
	  */
	virtual ref <const object> getPart() const = 0;

	/** Return the header of the attachment part.
	  *
	  * @return attachment part header or NULL if the attachment
	  * is not attached to a part
	  */
	virtual ref <const header> getHeader() const = 0;

protected:

	/** Generate the attachment in the specified body part.
	  *
	  * @param parent body part in which to generate the attachment
	  */
	virtual void generateIn(ref <bodyPart> parent) const = 0;
};


} // vmime


#endif // VMIME_ATTACHMENT_HPP_INCLUDED
