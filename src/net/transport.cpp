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

#include "vmime/net/transport.hpp"

#include "vmime/utility/stream.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/message.hpp"


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


void transport::send(ref <vmime::message> msg, utility::progressionListener* progress)
{
	// Extract expeditor
	mailbox expeditor;

	try
	{
		const mailboxField& from = dynamic_cast <const mailboxField&>
			(*msg->getHeader()->findField(fields::FROM));
		expeditor = from.getValue();
	}
	catch (exceptions::no_such_field&)
	{
		throw exceptions::no_expeditor();
	}

	// Extract recipients
	mailboxList recipients;

	try
	{
		const addressListField& to = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::TO));
		extractMailboxes(recipients, to.getValue());
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressListField& cc = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::CC));
		extractMailboxes(recipients, cc.getValue());
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressListField& bcc = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::BCC));
		extractMailboxes(recipients, bcc.getValue());
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


const transport::Type transport::getType() const
{
	return (TYPE_TRANSPORT);
}


} // net
} // vmime

