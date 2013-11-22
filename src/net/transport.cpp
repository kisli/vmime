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


transport::transport(shared_ptr <session> sess, const serviceInfos& infos, shared_ptr <security::authenticator> auth)
	: service(sess, infos, auth)
{
}


shared_ptr <headerField> transport::processHeaderField(shared_ptr <headerField> field)
{
	if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::BCC))
	{
		// Remove Bcc headers from the message, as required by the RFC.
		// Some SMTP server automatically strip this header (Postfix, qmail),
		// and others have an option for this (Exim).
		return null;
	}
	else if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::RETURN_PATH))
	{
		// RFC-2821: Return-Path header is added by the final transport system
		// that delivers the message to its recipient. Then, it should not be
		// transmitted to MSA.
   		return null;
	}
	else if (utility::stringUtils::isStringEqualNoCase(field->getName(), fields::ORIGINAL_RECIPIENT))
	{
		// RFC-2298: Delivering MTA may add the Original-Recipient header and
		// discard existing one; so, no need to send it.
		return null;
	}

	// Leave the header field as is
	return field;
}


void transport::processHeader(shared_ptr <header> header)
{
	if (header->getFieldCount() == 0)
		return;

	// Remove/replace fields
	for (size_t idx = header->getFieldCount() ; idx != 0 ; --idx)
	{
		shared_ptr <headerField> field = header->getFieldAt(idx - 1);
		shared_ptr <headerField> newField = processHeaderField(field);

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
		shared_ptr <mailbox> mbox = dynamicCast <mailbox>(list.getAddressAt(i)->clone());

		if (mbox != NULL)
			recipients.appendMailbox(mbox);
	}
}


void transport::send(shared_ptr <vmime::message> msg, utility::progressListener* progress)
{
	// Extract expeditor
	shared_ptr <mailbox> fromMbox =
		msg->getHeader()->findFieldValue <mailbox>(fields::FROM);

	if (!fromMbox)
		throw exceptions::no_expeditor();

	mailbox expeditor = *fromMbox;

	// Extract sender
	shared_ptr <mailbox> senderMbox =
		msg->getHeader()->findFieldValue <mailbox>(fields::SENDER);

	mailbox sender;

	if (!senderMbox)
		sender = expeditor;
	else
		sender = *senderMbox;

	// Extract recipients
	mailboxList recipients;

	// -- "To" field
	shared_ptr <addressList> addresses =
		msg->getHeader()->findFieldValue <addressList>(fields::TO);

	if (addresses)
		extractMailboxes(recipients, *addresses);

	// -- "Cc" field
	addresses =
		msg->getHeader()->findFieldValue <addressList>(fields::CC);

	if (addresses)
		extractMailboxes(recipients, *addresses);

	// -- "Bcc" field
	addresses =
		msg->getHeader()->findFieldValue <addressList>(fields::BCC);

	if (addresses)
		extractMailboxes(recipients, *addresses);

	// Process message header by removing fields that should be removed
	// before transmitting the message to MSA, and adding missing fields
	// which are required/recommended by the RFCs.
	shared_ptr <header> hdr = vmime::clone(msg->getHeader());
	processHeader(hdr);

	// To avoid cloning message body (too much overhead), use processed
	// header during the time we are generating the message to a stream.
	// Revert it back to original header after.
	struct XChangeMsgHeader
	{
		XChangeMsgHeader(shared_ptr <vmime::message> _msg,
		                 shared_ptr <vmime::header> _hdr)
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

		shared_ptr <vmime::message> msg;
		shared_ptr <vmime::header> hdr;
	} headerExchanger(msg, hdr);

	send(msg, expeditor, recipients, progress, sender);
}


void transport::send
	(shared_ptr <vmime::message> msg, const mailbox& expeditor, const mailboxList& recipients,
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

