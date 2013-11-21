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

#ifndef VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED
#define VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED


#include "vmime/mdn/MDNInfos.hpp"

#include "vmime/disposition.hpp"
#include "vmime/messageId.hpp"
#include "vmime/mailbox.hpp"


namespace vmime {
namespace mdn {


/** Holds information about a Message Disposition Notification (MDN)
  * that has been received.
  */

class VMIME_EXPORT receivedMDNInfos : public MDNInfos
{
public:

	receivedMDNInfos(const shared_ptr <const message> msg);
	receivedMDNInfos(const receivedMDNInfos& other);

	receivedMDNInfos& operator=(const receivedMDNInfos& other);


	const shared_ptr <const message> getMessage() const;

	/** Return the identifier of the message for which this MDN
	  * has been generated.
	  *
	  * @return original message-id
	  */
	const messageId getOriginalMessageId() const;

	/** Return information about the disposition.
	  *
	  * @return disposition information
	  */
	const disposition getDisposition() const;

	/** Return the Message Integrity Check (MIC), that is the value
	  * of the "Received-content-MIC" field.
	  *
	  * @return MIC hash value, or an empty string if not specified
	  */
	const string getContentMIC() const;

private:

	void copyFrom(const receivedMDNInfos& other);

	void extract();


	shared_ptr <const message> m_msg;

	disposition m_disp;
	messageId m_omid;
	string m_contentMIC;
};


} // mdn
} // vmime


#endif // VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED
