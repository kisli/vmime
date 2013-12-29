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

#ifndef VMIME_MESSAGEBUILDER_HPP_INCLUDED
#define VMIME_MESSAGEBUILDER_HPP_INCLUDED


#include "vmime/base.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/addressList.hpp"
#include "vmime/text.hpp"
#include "vmime/message.hpp"
#include "vmime/mediaType.hpp"
#include "vmime/attachment.hpp"
#include "vmime/textPart.hpp"
#include "vmime/bodyPart.hpp"


namespace vmime
{


/** A helper for building MIME messages.
  */

class VMIME_EXPORT messageBuilder
{
public:

	messageBuilder();
	~messageBuilder();

public:

	/** Return the expeditor of the message (From:).
	  *
	  * @return expeditor of the message
	  */
	const mailbox& getExpeditor() const;

	/** Set the expeditor of the message (From:).
	  *
	  * @param expeditor expeditor of the message
	  */
	void setExpeditor(const mailbox& expeditor);

	/** Return the recipients of the message (To:).
	  *
	  * return recipients of the message
	  */
	const addressList& getRecipients() const;

	/** Return the recipients of the message (To:).
	  *
	  * return recipients of the message
	  */
	addressList& getRecipients();

	/** Set the recipients of the message (To:).
	  *
	  * @param recipients list of recipients
	  */
	void setRecipients(const addressList& recipients);

	/** Return the copy recipients of the message (Cc:).
	  *
	  * @return copy recipients of the message
	  */
	const addressList& getCopyRecipients() const;

	/** Return the copy recipients of the message (Cc:).
	  *
	  * @return copy recipients of the message
	  */
	addressList& getCopyRecipients();

	/** Set the copy recipients of the message (Cc:).
	  *
	  * @param cc list of copy recipients
	  */
	void setCopyRecipients(const addressList& cc);

	/** Return the blind-copy recipients of the message (Bcc:).
	  *
	  * @return blind-copy recipients of the message
	  */
	const addressList& getBlindCopyRecipients() const;

	/** Return the blind-copy recipients of the message (Bcc:).
	  *
	  * @return blind-copy recipients of the message
	  */
	addressList& getBlindCopyRecipients();

	/** Set the blind-copy recipients of the message (Bcc:).
	  *
	  * @param bcc list of blind-copy recipients
	  */
	void setBlindCopyRecipients(const addressList& bcc);

	/** Return the subject of the message.
	  *
	  * @return subject of the message
	  */
	const text& getSubject() const;

	/** Set the subject of the message.
	  *
	  * @param subject message subject
	  */
	void setSubject(const text& subject);

	/** Attach a new object to the message.
	  * \deprecated Use messageBuilder::appendAttachment() instead.
	  *
	  * @param attach new attachment
	  */
	void attach(shared_ptr <attachment> attach);

	/** Attach a new object to the message.
	  *
	  * @param attach new attachment
	  */
	void appendAttachment(shared_ptr <attachment> attach);

	/** Remove the attachment at the specified position.
	  *
	  * @param pos position of the attachment to remove
	  */
	void removeAttachment(const size_t pos);

	/** Return the attachment at the specified position.
	  *
	  * @param pos position of the attachment
	  * @return attachment at the specified position
	  */
	const shared_ptr <const attachment> getAttachmentAt(const size_t pos) const;

	/** Return the attachment at the specified position.
	  *
	  * @param pos position of the attachment
	  * @return attachment at the specified position
	  */
	shared_ptr <attachment> getAttachmentAt(const size_t pos);

	/** Return the number of attachments in the message.
	  *
	  * @return number of attachments
	  */
	size_t getAttachmentCount() const;

	/** Return the list of attachments.
	  *
	  * @return list of attachments
	  */
	const std::vector <shared_ptr <const attachment> > getAttachmentList() const;

	/** Return the list of attachments.
	  *
	  * @return list of attachments
	  */
	const std::vector <shared_ptr <attachment> > getAttachmentList();

	/** Change the type of the text part and construct a new part.
	  *
	  * @param type media type of the text part
	  */
	void constructTextPart(const mediaType& type);

	/** Return the text part of the message.
	  *
	  * @return text part of the message
	  */
	shared_ptr <textPart> getTextPart();

	/** Construct a new message based on the information specified
	  * in this object.
	  *
	  * @return a new message
	  */
	shared_ptr <message> construct() const;

private:

	mailbox m_from;

	addressList m_to;
	addressList m_cc;
	addressList m_bcc;

	text m_subject;

	shared_ptr <textPart> m_textPart;

	std::vector <shared_ptr <attachment> > m_attach;
};


} // vmime


#endif // VMIME_MESSAGEBUILDER_HPP_INCLUDED
