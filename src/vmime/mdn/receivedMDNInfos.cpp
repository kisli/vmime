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

#include "vmime/mdn/receivedMDNInfos.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace mdn {


receivedMDNInfos::receivedMDNInfos(const shared_ptr <const message> msg)
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


const shared_ptr <const message> receivedMDNInfos::getMessage() const
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


const string receivedMDNInfos::getContentMIC() const
{
	return m_contentMIC;
}


void receivedMDNInfos::copyFrom(const receivedMDNInfos& other)
{
	m_msg = other.m_msg;
	m_omid = other.m_omid;
	m_disp = other.m_disp;
	m_contentMIC = other.m_contentMIC;
}


void receivedMDNInfos::extract()
{
	const shared_ptr <const body> bdy = m_msg->getBody();

	for (size_t i = 0 ; i < bdy->getPartCount() ; ++i)
	{
		const shared_ptr <const bodyPart> part = bdy->getPartAt(i);

		if (!part->getHeader()->hasField(fields::CONTENT_TYPE))
			continue;

		const mediaType& type = *part->getHeader()->ContentType()->getValue <mediaType>();

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

			shared_ptr <messageId> omid =
				fields.findFieldValue <messageId>(fields::ORIGINAL_MESSAGE_ID);

			if (omid)
				m_omid = *omid;

			shared_ptr <disposition> disp =
				fields.findFieldValue <disposition>(fields::DISPOSITION);

			if (disp)
				m_disp = *disp;

			shared_ptr <text> contentMIC =
				fields.findFieldValue <text>("Received-content-MIC");

			if (contentMIC)
				m_contentMIC = contentMIC->generate();
		}
	}
}


} // mdn
} // vmime
