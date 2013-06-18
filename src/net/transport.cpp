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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/transport.hpp"

#include "vmime/utility/stream.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/message.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/messageId.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace net {


transport::transport(ref <session> sess, const serviceInfos& infos, ref <security::authenticator> auth)
	: service(sess, infos, auth)
{
}


ref <headerField> transport::processHeaderField(ref <headerField> field)
{
	if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::BCC))
	{
		// Remove Bcc headers from the message, as required by the RFC.
		// Some SMTP server automatically strip this header (Postfix, qmail),
		// and others have an option for this (Exim).
		return NULL;
	}
	else if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::RETURN_PATH))
	{
		// RFC-2821: Return-Path header is added by the final transport system
		// that delivers the message to its recipient. Then, it should not be
		// transmitted to MSA.
   		return NULL;
	}
	else if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::ORIGINAL_RECIPIENT))
	{
		// RFC-2298: Delivering MTA may add the Original-Recipient header and
		// discard existing one; so, no need to send it.
		return NULL;
	}

	// Leave the header field as is
	return field;
}


void transport::processHeader(ref <header> header)
{
	if (header->getFieldCount() == 0)
		return;

	// Remove/replace fields
	for (size_t idx = header->getFieldCount() ; idx != 0 ; --idx)
	{
		ref <headerField> field = header->getFieldAt(idx - 1);
		ref <headerField> newField = processHeaderField(field);

		if (newField == NULL)
			header->removeField(field);
		else if (newField != field)
			header->replaceField(field, newField);
	}

	// Add missing header fields
	// -- Date
	if (!header->hasField(fields::DATE))
		header->Date()->setValue(datetime::now());

	// -- Mime-Version
	if (!header->hasField(fields::MIME_VERSION))
		header->MimeVersion()->setValue(string(SUPPORTED_MIME_VERSION));

	// -- Message-Id
	if (!header->hasField(fields::MESSAGE_ID))
		header->MessageId()->setValue(messageId::generateId());
}


static void extractMailboxes
	(mailboxList& recipients, const addressList& list)
{
	for (size_t i = 0 ; i < list.getAddressCount() ; ++i)
	{
		ref <mailbox> mbox = list.getAddressAt(i)->clone().dynamicCast <mailbox>();

		if (mbox != NULL)
			recipients.appendMailbox(mbox);
	}
}


void transport::send(ref <vmime::message> msg, utility::progressListener* progress)
{
	// Extract expeditor
	mailbox expeditor;

	try
	{
		const mailbox& mbox = *msg->getHeader()->findField(fields::FROM)->
			getValue().dynamicCast <const mailbox>();

		expeditor = mbox;
	}
	catch (exceptions::no_such_field&)
	{
		throw exceptions::no_expeditor();
	}

	// Extract sender
	mailbox sender;

	try
	{
		const mailbox& mbox = *msg->getHeader()->findField(fields::SENDER)->
			getValue().dynamicCast <const mailbox>();

		sender = mbox;
	}
	catch (exceptions::no_such_field&)
	{
		sender = expeditor;
	}

	// Extract recipients
	mailboxList recipients;

	try
	{
		const addressList& to = *msg->getHeader()->findField(fields::TO)->
			getValue().dynamicCast <const addressList>();

		extractMailboxes(recipients, to);
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressList& cc = *msg->getHeader()->findField(fields::CC)->
			getValue().dynamicCast <const addressList>();

		extractMailboxes(recipients, cc);
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressList& bcc = *msg->getHeader()->findField(fields::BCC)->
			getValue().dynamicCast <const addressList>();

		extractMailboxes(recipients, bcc);
	}
	catch (exceptions::no_such_field&) { }

	// Process message header by removing fields that should be removed
	// before transmitting the message to MSA, and adding missing fields
	// which are required/recommended by the RFCs.
	ref <header> hdr = msg->getHeader()->clone().dynamicCast <header>();
	processHeader(hdr);

	// To avoid cloning message body (too much overhead), use processed
	// header during the time we are generating the message to a stream.
	// Revert it back to original header after.
	struct XChangeMsgHeader
	{
		XChangeMsgHeader(vmime::ref <vmime::message> _msg,
		                 vmime::ref <vmime::header> _hdr)
			: msg(_msg), hdr(msg->getHeader())
		{
			// Set new header
			msg->setHeader(_hdr);
		}

		~XChangeMsgHeader()
		{
			// Revert original header
			msg->setHeader(hdr);
		}

	private:

		vmime::ref <vmime::message> msg;
		vmime::ref <vmime::header> hdr;
	} headerExchanger(msg, hdr);

	send(msg, expeditor, recipients, progress, sender);
}


void transport::send
	(ref <vmime::message> msg, const mailbox& expeditor, const mailboxList& recipients,
	 utility::progressListener* progress, const mailbox& sender)
{
	// Generate the message, "stream" it and delegate the sending
	// to the generic send() function.
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str(oss.str());

	utility::inputStreamStringAdapter isAdapter(str);

	send(expeditor, recipients, isAdapter, str.length(), progress, sender);
}


transport::Type transport::getType() const
{
	return (TYPE_TRANSPORT);
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

