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

#ifndef VMIME_MESSAGEPARSER_HPP_INCLUDED
#define VMIME_MESSAGEPARSER_HPP_INCLUDED


#include "base.hpp"

#include "message.hpp"
#include "attachment.hpp"

#include "textPart.hpp"


namespace vmime
{


/** A helper for parsing MIME messages.
  */

class messageParser
{
public:

	messageParser(const string& buffer);
	messageParser(const message& msg);
	~messageParser();

public:

	// Expeditor and recipients
	const mailbox& expeditor() const { return (m_from); }

	const addressList& recipients() const { return (m_to); }
	const addressList& copyRecipients() const { return (m_cc); }
	const addressList& blindCopyRecipients() const { return (m_bcc); }

	// Subject
	const text& subject() const { return (m_subject); }

	// Date
	const datetime& date() const { return (m_date); }

	// Attachments
	const std::vector <attachment*>& attachments() const { return (m_attach); }
	const contentDispositionField* attachmentInfo(attachment* a) const;

	// Text parts
	const std::vector <textPart*>& textParts() const { return (m_textParts); }

protected:

	mailbox m_from;

	addressList m_to;
	addressList m_cc;
	addressList m_bcc;

	text m_subject;

	datetime m_date;

	std::vector <attachment*> m_attach;
	std::map <attachment*, contentDispositionField*> m_attachInfo;

	std::vector <textPart*> m_textParts;

	void parse(const message& msg);

	void findAttachments(const bodyPart& part);

	void findTextParts(const bodyPart& msg, const bodyPart& part);
	bool findSubTextParts(const bodyPart& msg, const bodyPart& part);
};


} // vmime


#endif // VMIME_MESSAGEPARSER_HPP_INCLUDED
