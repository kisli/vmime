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

#ifndef VMIME_DEFAULTATTACHMENT_HPP_INCLUDED
#define VMIME_DEFAULTATTACHMENT_HPP_INCLUDED


#include "attachment.hpp"
#include "encoding.hpp"


namespace vmime
{


class defaultAttachment : public attachment
{
protected:

	// For use in derived classes.
	defaultAttachment();

public:

	defaultAttachment(const contentHandler& data, const encoding& enc, const mediaType& type, const text& desc = NULL_TEXT);
	defaultAttachment(const contentHandler& data, const mediaType& type, const text& desc = NULL_TEXT);
	defaultAttachment(const defaultAttachment& attach);

	defaultAttachment& operator=(const defaultAttachment& attach);

	const mediaType& getType() const;
	const text& getDescription() const;
	const contentHandler& getData() const;
	const encoding& getEncoding() const;

protected:

	mediaType m_type;           // Media type (eg. "application/octet-stream")
	text m_desc;                // Description (eg. "The image you requested")
	contentHandler m_data;      // Attachment data (eg. the file contents)
	encoding m_encoding;        // Encoding

private:

	// No need to override "generateIn", use "generatePart" instead (see below).
	void generateIn(bodyPart& parent) const;

protected:

	virtual void generatePart(bodyPart& part) const;
};


} // vmime


#endif // VMIME_DEFAULTATTACHMENT_HPP_INCLUDED
