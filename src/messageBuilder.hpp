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

#ifndef VMIME_MESSAGEBUILDER_HPP_INCLUDED
#define VMIME_MESSAGEBUILDER_HPP_INCLUDED


#include "base.hpp"

#include "mailbox.hpp"
#include "addressList.hpp"
#include "text.hpp"
#include "message.hpp"
#include "mediaType.hpp"
#include "attachment.hpp"
#include "textPart.hpp"
#include "bodyPart.hpp"


namespace vmime
{


/** A helper for building MIME messages.
  */

class messageBuilder
{
public:

	messageBuilder();
	~messageBuilder();

public:

	// Expeditor and recipients
	const mailbox& expeditor() const { return (m_from); }
	mailbox& expeditor() { return (m_from); }

	const addressList& recipients() const { return (m_to); }
	addressList& recipients() { return (m_to); }

	const addressList& copyRecipients() const { return (m_cc); }
	addressList& copyRecipients() { return (m_cc); }

	const addressList& blindCopyRecipients() const { return (m_bcc); }
	addressList& blindCopyRecipients() { return (m_bcc); }

	// Subject
	const text& subject() const { return (m_subject); }
	text& subject() { return (m_subject); }

	// Attachements
	void attach(attachment* attach);
	const std::vector <attachment*>& attachments() const { return (m_attach); }

	// Text parts
	void constructTextPart(const mediaType& type);
	class textPart& textPart();

	// Construction
	message* construct() const;

protected:

	mailbox m_from;

	addressList m_to;
	addressList m_cc;
	addressList m_bcc;

	text m_subject;

	class textPart* m_textPart;

	std::vector <attachment*> m_attach;
};


} // vmime


#endif // VMIME_MESSAGEBUILDER_HPP_INCLUDED
