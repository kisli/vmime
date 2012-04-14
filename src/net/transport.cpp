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

#include "vmime/net/transport.hpp"

#include "vmime/utility/stream.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/message.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace net {


transport::transport(ref <session> sess, const serviceInfos& infos, ref <security::authenticator> auth)
	: service(sess, infos, auth)
{
}


static void extractMailboxes
	(mailboxList& recipients, const addressList& list)
{
	for (int i = 0 ; i < list.getAddressCount() ; ++i)
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

	// Remove BCC headers from the message we're about to send, as required by the RFC.
	// Some SMTP server automatically strip this header (Postfix, qmail), and others
	// have an option for this (Exim).
	try
	{
		ref <headerField> bcc = msg->getHeader()->findField(fields::BCC);
		msg->getHeader()->removeField(bcc);
	}
	catch (exceptions::no_such_field&) { }

	// Generate the message, "stream" it and delegate the sending
	// to the generic send() function.
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str(oss.str());

	utility::inputStreamStringAdapter isAdapter(str);

	send(expeditor, recipients, isAdapter, str.length(), progress);
}


transport::Type transport::getType() const
{
	return (TYPE_TRANSPORT);
}


} // net
} // vmime

