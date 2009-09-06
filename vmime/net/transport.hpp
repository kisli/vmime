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

#ifndef VMIME_NET_TRANSPORT_HPP_INCLUDED
#define VMIME_NET_TRANSPORT_HPP_INCLUDED


#include "vmime/net/service.hpp"
#include "vmime/utility/stream.hpp"

#include "vmime/mailboxList.hpp"


namespace vmime {

class message;
class mailbox;
class mailboxList;

namespace net {


/** A transport service.
  * Encapsulate protocols that can send messages.
  */

class transport : public service
{
protected:

	transport(ref <session> sess, const serviceInfos& infos, ref <security::authenticator> auth);

public:

	/** Send a message over this transport service.
	  *
	  * @param msg message to send
	  * @param progress progress listener, or NULL if not used
	  */
	virtual void send(ref <vmime::message> msg, utility::progressListener* progress = NULL);

	/** Send a message over this transport service.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param is input stream provding message data (header + body)
	  * @param size size of the message data
	  * @param progress progress listener, or NULL if not used
	  */
	virtual void send(const mailbox& expeditor, const mailboxList& recipients, utility::inputStream& is, const utility::stream::size_type size, utility::progressListener* progress = NULL) = 0;


	Type getType() const;
};


} // net
} // vmime


#endif // VMIME_NET_TRANSPORT_HPP_INCLUDED
