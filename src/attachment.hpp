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

#ifndef VMIME_ATTACHMENT_HPP_INCLUDED
#define VMIME_ATTACHMENT_HPP_INCLUDED


#include "base.hpp"

#include "bodyPart.hpp"
#include "mediaType.hpp"
#include "text.hpp"
#include "contentHandler.hpp"
#include "encoding.hpp"


namespace vmime
{


/** Base class for all types of attachment.
  */

class attachment
{
	friend class messageBuilder;
	friend class messageParser;

protected:

	attachment() { }

public:

	virtual ~attachment() { }

	/** Return the media type of this attachment.
	  * @return content type of the attachment
	  */
	virtual const mediaType& getType() const = 0;

	/** Return the description of this attachment.
	  * @return attachment description
	  */
	virtual const text& getDescription() const = 0;

	/** Return the data contained in this attachment.
	  * @return attachment data
	  */
	virtual const contentHandler& getData() const = 0;

	/** Return the encoding used for this attachment.
	  * @return attachment data encoding
	  */
	virtual const encoding& getEncoding() const = 0;

	/** Generate the attachment in the specified body part.
	  * @param parent body part in which to generate the attachment
	  */
	virtual void generateIn(bodyPart& parent) const = 0;
};


} // vmime


#endif // VMIME_ATTACHMENT_HPP_INCLUDED
