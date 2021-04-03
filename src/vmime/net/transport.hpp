//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_TRANSPORT_HPP_INCLUDED
#define VMIME_NET_TRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/service.hpp"
#include "vmime/utility/stream.hpp"

#include "vmime/mailboxList.hpp"


namespace vmime {

class header;
class headerField;
class message;
class mailbox;
class mailboxList;

namespace net {


/** A transport service.
  * Encapsulate protocols that can send messages.
  */
class VMIME_EXPORT transport : public service {

protected:

	transport(
		const shared_ptr <session>& sess,
		const serviceInfos& infos,
		const shared_ptr <security::authenticator>& auth
	);

public:

	/** Holds a set of options that can be passed to send() methods.
	  */
	class VMIME_EXPORT sendOptions {

	public:

		sendOptions();
		virtual ~sendOptions();
	};

	/** Send a message over this transport service.
	  * The default implementation simply generates the whole message into
	  * a string buffer and "streams" it via a inputStreamStringAdapter.
	  *
	  * @param msg message to send
	  * @param progress progress listener, or NULL if not used
	  * @param options sending options
	  */
	virtual void send(
		const shared_ptr <vmime::message>& msg,
		utility::progressListener* progress = NULL,
		const sendOptions& options = sendOptions()
	);

	/** Send a message over this transport service.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param is input stream providing message data (header + body)
	  * @param size size of the message data
	  * @param progress progress listener, or NULL if not used
	  * @param sender envelope sender (if empty, expeditor will be used)
	  * @param options sending options
	  */
	virtual void send(
		const mailbox& expeditor,
		const mailboxList& recipients,
		utility::inputStream& is,
		const size_t size,
		utility::progressListener* progress = NULL,
		const mailbox& sender = mailbox(),
		const sendOptions& options = sendOptions()
	) = 0;

	/** Send a message over this transport service.
	  * The default implementation simply generates the whole message into
	  * a string buffer and "streams" it via a inputStreamStringAdapter.
	  *
	  * @param msg message to send
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param progress progress listener, or NULL if not used
	  * @param sender envelope sender (if empty, expeditor will be used)
	  * @param options sending options
	  */
	virtual void send(
		const shared_ptr <vmime::message>& msg,
		const mailbox& expeditor,
		const mailboxList& recipients,
		utility::progressListener* progress = NULL,
		const mailbox& sender = mailbox(),
		const sendOptions& options = sendOptions()
	);


	Type getType() const;

protected:

	/** Called by processHeader().
	  * Decides what to do with the specified header field.
	  *
	  * @return NULL if the header should be removed, a reference to a new headerField
	  * if the field is to be replaced, or a reference to the same headerField
	  * that was passed if the field should be left as is
	  */
	shared_ptr <headerField> processHeaderField(const shared_ptr <headerField>& field);

	/** Prepares the header before transmitting the message.
	  * Removes headers that should not be present (eg. "Bcc", "Return-Path"),
	  * or adds missing headers that are required/recommended by the RFCs.
	  * The header is modified inline.
	  *
	  * @param header headers to process
	  */
	void processHeader(const shared_ptr <header>& header);
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_TRANSPORT_HPP_INCLUDED
