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

#ifndef VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED
#define VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED


#include "vmime/mdn/MDNInfos.hpp"
#include "vmime/disposition.hpp"


namespace vmime {
namespace mdn {


/** Holds information about a Message Disposition Notification (MDN)
  * that has been received.
  */

class receivedMDNInfos : public MDNInfos
{
public:

	receivedMDNInfos(const message* msg);
	receivedMDNInfos(const receivedMDNInfos& other);

	receivedMDNInfos& operator=(const receivedMDNInfos& other);


	const message* getMessage() const;

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

private:

	void copyFrom(const receivedMDNInfos& other);

	void extract();


	const message* m_msg;

	disposition m_disp;
	messageId m_omid;
};


} // mdn
} // vmime


#endif // VMIME_MDN_RECEIVEDMDNINFOS_HPP_INCLUDED
