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

class VMIME_EXPORT textPart : public object
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
	virtual const shared_ptr <const contentHandler> getText() const = 0;

	/** Set the text contained in the part.
	  *
	  * @param text text of the part
	  */
	virtual void setText(shared_ptr <contentHandler> text) = 0;

	/** Return the actual body parts this text part is composed of.
	  * For example, HTML parts are composed of two parts: one "text/html"
	  * part, and the plain text part "text/plain".
	  *
	  * @return number of body parts
	  */
	virtual size_t getPartCount() const = 0;

	/** Generate the text part(s) into the specified message.
	  *
	  * @param message the message
	  * @param parent body part into which generate this part
	  */
	virtual void generateIn(shared_ptr <bodyPart> message, shared_ptr <bodyPart> parent) const = 0;

	/** Parse the text part(s) from the specified message.
	  *
	  * @param message message containing the text part
	  * @param parent part containing the text part
	  * @param textPart actual text part
	  */
	virtual void parse(shared_ptr <const bodyPart> message, shared_ptr <const bodyPart> parent, shared_ptr <const bodyPart> textPart) = 0;
};


} // vmime


#endif // VMIME_TEXTPART_HPP_INCLUDED
