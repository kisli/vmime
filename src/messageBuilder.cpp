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

#include "vmime/messageBuilder.hpp"

#include "vmime/dateTime.hpp"
#include "vmime/textPartFactory.hpp"


namespace vmime
{


messageBuilder::messageBuilder()
{
	// By default there is one text part of type "text/plain"
	constructTextPart(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
}


messageBuilder::~messageBuilder()
{
}


ref <message> messageBuilder::construct() const
{
	// Create a new message
	ref <message> msg = vmime::create <message>();

	// Generate the header fields
	msg->getHeader()->Subject()->setValue(m_subject);

	if (((m_to.isEmpty()) || (m_to.getAddressAt(0)->isEmpty() && !m_to.getAddressAt(0)->isGroup())) &&
	    (m_cc.isEmpty() || m_cc.getAddressAt(0)->isEmpty()) &&
	    (m_bcc.isEmpty() || m_bcc.getAddressAt(0)->isEmpty()))
	{
		throw exceptions::no_recipient();
	}

	if (!m_from.isEmpty())
		msg->getHeader()->From()->setValue(m_from);

	if (!m_to.isEmpty())
		msg->getHeader()->To()->setValue(m_to);

	if (!m_cc.isEmpty())
		msg->getHeader()->Cc()->setValue(m_cc);

	if (!m_bcc.isEmpty())
		msg->getHeader()->Bcc()->setValue(m_bcc);

	// Add a "Date" field
	msg->getHeader()->Date()->setValue(datetime::now());

	// Add a "Mime-Version" header field
	msg->getHeader()->MimeVersion()->setValue(string(SUPPORTED_MIME_VERSION));

	// If there is one or more attachments (or other parts that are
	// not "text/...") and if there is more than one parts for the
	// text part, we generate these text parts into a sub-part:
	//
	// [message]
	// |
	// +-- multipart/mixed
	//     |
	//     +-- multipart/alternative
	//     |   |
	//     |   +-- text part #1  (eg. plain text "text/plain")
	//     |   +-- text part #2  (eg. HTML       "text/html")
	//     |   +-- ...
	//     |
	//     +-- application/octet-stream (attachment #1)
	//     |
	//     +-- ... (other attachments/parts)
	//
	if (!m_attach.empty() && m_textPart->getPartCount() > 1)
	{
		// Set parent part (message) to "multipart/mixed"
		msg->getHeader()->ContentType()->setValue
			(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));

		// Create a sub-part "multipart/alternative" for text parts
		ref <bodyPart> subPart = vmime::create <bodyPart>();
		msg->getBody()->appendPart(subPart);

		subPart->getHeader()->ContentType()->setValue
			(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE));

		// Generate the text parts into this sub-part (normally, this
		// sub-part will have the "multipart/alternative" content-type...)
		m_textPart->generateIn(msg, subPart);
	}
	else
	{
		// Generate the text part(s) directly into the message
		m_textPart->generateIn(msg, msg);

		// If any attachment, set message content-type to "multipart/mixed"
		if (!m_attach.empty())
		{
			msg->getHeader()->ContentType()->setValue
				(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));
		}
		// Else, set it to "multipart/alternative" if there are more than one text part.
		else if (m_textPart->getPartCount() > 1)
		{
			msg->getHeader()->ContentType()->setValue
				(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE));
		}
	}

	// Generate the attachments
	if (!m_attach.empty())
	{
		for (std::vector <ref <attachment> >::const_iterator a = m_attach.begin() ;
		     a != m_attach.end() ; ++a)
		{
			(*a)->generateIn(msg);
		}
	}

	// If there is only one part in the message, move it into the message
	// (hence, the message will not be multipart...)
	if (msg->getBody()->getPartCount() == 1)
	{
		const bodyPart& part = *msg->getBody()->getPartAt(0);

		// Make a full copy of the body, otherwise the copyFrom() will delete the body we're copying
		ref <body> bodyCopy = part.getBody()->clone().dynamicCast <body>();

		// First, copy (and replace) the header fields
		const std::vector <ref <const headerField> > fields = part.getHeader()->getFieldList();

		for (std::vector <ref <const headerField> >::const_iterator it = fields.begin() ;
		     it != fields.end() ; ++it)
		{
			*(msg->getHeader()->getField((*it)->getName())) = **it;
		}

		// Second, copy the body contents and sub-parts (this also remove
		// the body part we are copying...)
		msg->getBody()->copyFrom(*bodyCopy);
	}

	return (msg);
}


void messageBuilder::attach(ref <attachment> attach)
{
	appendAttachment(attach);
}


void messageBuilder::appendAttachment(ref <attachment> attach)
{
	m_attach.push_back(attach);
}


void messageBuilder::constructTextPart(const mediaType& type)
{
	ref <textPart> part = NULL;

	try
	{
		part = textPartFactory::getInstance()->create(type);
	}
	catch (exceptions::no_factory_available& e)
	{
		throw;
	}

	m_textPart = part;
}


ref <textPart> messageBuilder::getTextPart()
{
	return (m_textPart);
}


const mailbox& messageBuilder::getExpeditor() const
{
	return (m_from);
}


void messageBuilder::setExpeditor(const mailbox& expeditor)
{
	m_from = expeditor;
}


const addressList& messageBuilder::getRecipients() const
{
	return (m_to);
}


addressList& messageBuilder::getRecipients()
{
	return (m_to);
}


void messageBuilder::setRecipients(const addressList& recipients)
{
	m_to = recipients;
}


const addressList& messageBuilder::getCopyRecipients() const
{
	return (m_cc);
}


addressList& messageBuilder::getCopyRecipients()
{
	return (m_cc);
}


void messageBuilder::setCopyRecipients(const addressList& cc)
{
	m_cc = cc;
}


const addressList& messageBuilder::getBlindCopyRecipients() const
{
	return (m_bcc);
}


addressList& messageBuilder::getBlindCopyRecipients()
{
	return (m_bcc);
}


void messageBuilder::setBlindCopyRecipients(const addressList& bcc)
{
	m_bcc = bcc;
}


const text& messageBuilder::getSubject() const
{
	return (m_subject);
}


void messageBuilder::setSubject(const text& subject)
{
	m_subject = subject;
}


void messageBuilder::removeAttachment(const int pos)
{
	m_attach.erase(m_attach.begin() + pos);
}


const ref <const attachment> messageBuilder::getAttachmentAt(const int pos) const
{
	return (m_attach[pos]);
}


ref <attachment> messageBuilder::getAttachmentAt(const int pos)
{
	return (m_attach[pos]);
}


int messageBuilder::getAttachmentCount() const
{
	return (m_attach.size());
}


const std::vector <ref <const attachment> > messageBuilder::getAttachmentList() const
{
	std::vector <ref <const attachment> > res;

	res.reserve(m_attach.size());

	for (std::vector <ref <attachment> >::const_iterator it = m_attach.begin() ;
	     it != m_attach.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


const std::vector <ref <attachment> > messageBuilder::getAttachmentList()
{
	return (m_attach);
}


} // vmime
