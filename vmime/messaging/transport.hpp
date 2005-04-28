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

#ifndef VMIME_MESSAGING_TRANSPORT_HPP_INCLUDED
#define VMIME_MESSAGING_TRANSPORT_HPP_INCLUDED


#include "vmime/messaging/service.hpp"
#include "vmime/utility/stream.hpp"


namespace vmime {

class message;
class mailbox;
class mailboxList;

namespace messaging {


/** A transport service.
  * Encapsulate protocols that can send messages.
  */

class transport : public service
{
protected:

	transport(session* sess, const serviceInfos& infos, authenticator* auth);

public:

	/** Send a message over this transport service.
	  *
	  * @param msg message to send
	  * @param progress progression listener, or NULL if not used
	  */
	virtual void send(vmime::message* msg, utility::progressionListener* progress = NULL);

	/** Send a message over this transport service.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param is input stream provding message data (header + body)
	  * @param size size of the message data
	  * @param progress progression listener, or NULL if not used
	  */
	virtual void send(const mailbox& expeditor, const mailboxList& recipients, utility::inputStream& is, const utility::stream::size_type size, utility::progressionListener* progress = NULL) = 0;


	const Type getType() const;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_TRANSPORT_HPP_INCLUDED
