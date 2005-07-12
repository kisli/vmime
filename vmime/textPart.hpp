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

#ifndef VMIME_TEXTPART_HPP_INCLUDED
#define VMIME_TEXTPART_HPP_INCLUDED


#include "vmime/bodyPart.hpp"

#include "vmime/mediaType.hpp"
#include "vmime/charset.hpp"
#include "vmime/contentHandler.hpp"


namespace vmime
{


/** Generic text part.
  */

class textPart : public object
{
	friend class textPartFactory;
	friend class messageBuilder; // for generateIn, getPartCount
	friend class messageParser;  // for parse

public:

	virtual ~textPart() { }

	/** Return the type of text part (eg: "text/html").
	  *
	  * @return type of text part
	  */
	virtual const mediaType getType() const = 0;

	/** Return the charset used to encode text in the
	  * text part.
	  *
	  * @return text charset
	  */
	virtual const charset& getCharset() const = 0;

	/** Set the charset used to encode text in the
	  * text part.
	  *
	  * @param ch text charset
	  */
	virtual void setCharset(const charset& ch) = 0;

	/** Return the text contained in the part.
	  *
	  * @return text of the part
	  */
	virtual const ref <const contentHandler> getText() const = 0;

	/** Set the text contained in the part.
	  *
	  * @param text text of the part
	  */
	virtual void setText(ref <contentHandler> text) = 0;

protected:

	virtual const int getPartCount() const = 0;

	virtual void generateIn(bodyPart& message, bodyPart& parent) const = 0;
	virtual void parse(const bodyPart& message, const bodyPart& parent, const bodyPart& textPart) = 0;
};


} // vmime


#endif // VMIME_TEXTPART_HPP_INCLUDED
