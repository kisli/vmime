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

#ifndef VMIME_MESSAGEPARSER_HPP_INCLUDED
#define VMIME_MESSAGEPARSER_HPP_INCLUDED


#include "vmime/base.hpp"

#include "vmime/message.hpp"
#include "vmime/attachment.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/addressList.hpp"
#include "vmime/dateTime.hpp"

#include "vmime/textPart.hpp"


namespace vmime
{


/** A helper for parsing MIME messages.
  */

class messageParser
{
public:

	messageParser(const string& buffer);
	messageParser(ref <const message> msg);
	~messageParser();

public:

	/** Return the expeditor of the message (From:).
	  *
	  * @return expeditor of the message
	  */
	const mailbox& getExpeditor() const;

	/** Return the recipients of the message (To:).
	  *
	  * return recipients of the message
	  */
	const addressList& getRecipients() const;

	/** Return the copy recipients of the message (Cc:).
	  *
	  * @return copy recipients of the message
	  */
	const addressList& getCopyRecipients() const;

	/** Return the blind-copy recipients of the message (Bcc:).
	  *
	  * @return blind-copy recipients of the message
	  */
	const addressList& getBlindCopyRecipients() const;

	/** Return the subject of the message.
	  *
	  * @return subject of the message
	  */
	const text& getSubject() const;

	/** Return the date of the message.
	  *
	  * @return date of the message
	  */
	const datetime& getDate() const;

	/** Return the number of attachments in the message.
	  *
	  * @return number of attachments
	  */
	int getAttachmentCount() const;

	/** Return the attachment at the specified position.
	  *
	  * @param pos position of the attachment
	  * @return attachment at position 'pos'
	  */
	const ref <const attachment> getAttachmentAt(const int pos) const;

	/** Return the attachments of the message.
	  *
	  * @return list of attachments in the message
	  */
	const std::vector <ref <const attachment> > getAttachmentList() const;

	/** Return the text parts of the message.
	  *
	  * @return list of text parts in the message
	  */
	const std::vector <ref <const textPart> > getTextPartList() const;

	/** Return the number of text parts in the message.
	  *
	  * @return number of text parts
	  */
	int getTextPartCount() const;

	/** Return the text part at the specified position.
	  *
	  * @param pos position of the text part
	  * @return text part at position 'pos'
	  */
	const ref <const textPart> getTextPartAt(const int pos) const;

private:

	mailbox m_from;

	addressList m_to;
	addressList m_cc;
	addressList m_bcc;

	text m_subject;

	datetime m_date;

	std::vector <ref <const attachment> > m_attach;

	std::vector <ref <textPart> > m_textParts;

	void parse(ref <const message> msg);

	void findAttachments(ref <const message> msg);

	void findTextParts(ref <const bodyPart> msg, ref <const bodyPart> part);
	bool findSubTextParts(ref <const bodyPart> msg, ref <const bodyPart> part);
};


} // vmime


#endif // VMIME_MESSAGEPARSER_HPP_INCLUDED
