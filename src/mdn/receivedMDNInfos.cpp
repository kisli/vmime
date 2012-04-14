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

#include "vmime/mdn/receivedMDNInfos.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace mdn {


receivedMDNInfos::receivedMDNInfos(const ref <const message> msg)
	: m_msg(msg)
{
	extract();
}


receivedMDNInfos::receivedMDNInfos(const receivedMDNInfos& other)
	: MDNInfos()
{
	copyFrom(other);
}


receivedMDNInfos& receivedMDNInfos::operator=(const receivedMDNInfos& other)
{
	copyFrom(other);
	return (*this);
}


const ref <const message> receivedMDNInfos::getMessage() const
{
	return (m_msg);
}


const messageId receivedMDNInfos::getOriginalMessageId() const
{
	return (m_omid);
}


const disposition receivedMDNInfos::getDisposition() const
{
	return (m_disp);
}


void receivedMDNInfos::copyFrom(const receivedMDNInfos& other)
{
	m_msg = other.m_msg;
	m_omid = other.m_omid;
	m_disp = other.m_disp;
}


void receivedMDNInfos::extract()
{
	const ref <const body> bdy = m_msg->getBody();

	for (int i = 0 ; i < bdy->getPartCount() ; ++i)
	{
		const ref <const bodyPart> part = bdy->getPartAt(i);

		if (!part->getHeader()->hasField(fields::CONTENT_TYPE))
			continue;

		const mediaType& type = *part->getHeader()->ContentType()->
			getValue().dynamicCast <const mediaType>();

		// Extract from second part (message/disposition-notification)
		if (type.getType() == vmime::mediaTypes::MESSAGE &&
		    type.getSubType() == vmime::mediaTypes::MESSAGE_DISPOSITION_NOTIFICATION)
		{
			std::ostringstream oss;
			utility::outputStreamAdapter vos(oss);

			part->getBody()->getContents()->extract(vos);

			// Body actually contains fields
			header fields;
			fields.parse(oss.str());

			try { m_omid = *fields.OriginalMessageId()->getValue().dynamicCast <const messageId>(); }
			catch (exceptions::no_such_field&) { /* Ignore */ }

			try { m_disp = *fields.Disposition()->getValue().dynamicCast <const disposition>(); }
			catch (exceptions::no_such_field&) { /* Ignore */ }
		}
	}
}


} // mdn
} // vmime
