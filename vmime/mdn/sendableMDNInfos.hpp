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

#ifndef VMIME_MDN_SENDABLEMDNINFOS_HPP_INCLUDED
#define VMIME_MDN_SENDABLEMDNINFOS_HPP_INCLUDED


#include "vmime/mdn/MDNInfos.hpp"


namespace vmime {
namespace mdn {


/** Holds information about a Message Disposition Notifications (MDN)
  * that is to be sent.
  */

class sendableMDNInfos : public MDNInfos
{
public:

	sendableMDNInfos(const ref <const message> msg, const mailbox& mbox);
	sendableMDNInfos(const sendableMDNInfos& other);

	sendableMDNInfos& operator=(const sendableMDNInfos& other);

	const ref <const message> getMessage() const;

	/** Return the recipient of the MDN (the mailbox that will receive
	  * the notification message).
	  *
	  * @return recipient of the MDN
	  */
	const mailbox& getRecipient() const;

private:

	void copyFrom(const sendableMDNInfos& other);


	ref <const message> m_msg;
	mailbox m_mailbox;
};


} // mdn
} // vmime


#endif // VMIME_MDN_SENDABLEMDNINFOS_HPP_INCLUDED
