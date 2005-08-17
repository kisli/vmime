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

#ifndef VMIME_DEFAULTATTACHMENT_HPP_INCLUDED
#define VMIME_DEFAULTATTACHMENT_HPP_INCLUDED


#include "vmime/attachment.hpp"
#include "vmime/encoding.hpp"


namespace vmime
{


/** Default implementation for attachments.
  */

class defaultAttachment : public attachment
{
protected:

	// For use in derived classes.
	defaultAttachment();

public:

	defaultAttachment(ref <const contentHandler> data, const encoding& enc, const mediaType& type, const text& desc = NULL_TEXT, const word& name = NULL_WORD);
	defaultAttachment(ref <const contentHandler> data, const mediaType& type, const text& desc = NULL_TEXT, const word& name = NULL_WORD);
	defaultAttachment(const defaultAttachment& attach);

	~defaultAttachment();

	defaultAttachment& operator=(const defaultAttachment& attach);

	const mediaType& getType() const;
	const text& getDescription() const;
	const word& getName() const;
	const ref <const contentHandler> getData() const;
	const encoding& getEncoding() const;

protected:

	mediaType m_type;                   /**< Media type (eg. "application/octet-stream") */
	text m_desc;                        /**< Description (eg. "The image you requested") */
	ref <const contentHandler> m_data;  /**< Attachment data (eg. the file contents) */
	encoding m_encoding;                /**< Encoding */
	word m_name;                        /**< Name/filename (eg. "sunset.jpg") */

private:

	// No need to override "generateIn", use "generatePart" instead (see below).
	void generateIn(bodyPart& parent) const;

protected:

	virtual void generatePart(bodyPart& part) const;
};


} // vmime


#endif // VMIME_DEFAULTATTACHMENT_HPP_INCLUDED
