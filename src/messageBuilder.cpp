//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/messageBuilder.hpp"

#include "vmime/textPartFactory.hpp"


namespace vmime
{


messageBuilder::messageBuilder()
	: m_textPart(NULL)
{
	// By default there is one text part of type "text/plain"
	constructTextPart(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
}


messageBuilder::~messageBuilder()
{
	delete (m_textPart);

	free_container(m_attach);
}


message* messageBuilder::construct() const
{
	// Create a new message
	message* msg = new message;

	// Generate the header fields
	msg->getHeader()->Subject().setValue(m_subject);

	if (m_from.isEmpty())
		throw exceptions::no_expeditor();

	if (m_to.isEmpty() || m_to.getAddressAt(0)->isEmpty())
		throw exceptions::no_recipient();

	msg->getHeader()->From().setValue(m_from);
	msg->getHeader()->To().setValue(m_to);

	if (!m_cc.isEmpty())
		msg->getHeader()->Cc().setValue(m_cc);

	if (!m_bcc.isEmpty())
		msg->getHeader()->Bcc().setValue(m_bcc);

	// Add a "Date" field
	msg->getHeader()->Date().setValue(datetime::now());

	// Add a "Mime-Version" header field
	msg->getHeader()->MimeVersion().setValue(string(MIME_VERSION));

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
		msg->getHeader()->ContentType().setValue
			(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));

		// Create a sub-part "multipart/alternative" for text parts
		bodyPart* subPart = new bodyPart;
		msg->getBody()->appendPart(subPart);

		subPart->getHeader()->ContentType().setValue
			(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE));

		// Generate the text parts into this sub-part (normally, this
		// sub-part will have the "multipart/alternative" content-type...)
		m_textPart->generateIn(*msg, *subPart);
	}
	else
	{
		// Generate the text part(s) directly into the message
		m_textPart->generateIn(*msg, *msg);

		// If any attachment, set message content-type to "multipart/mixed"
		if (!m_attach.empty())
		{
			msg->getHeader()->ContentType().setValue
				(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));
		}
		// Else, set it to "multipart/alternative" if there are more than one text part.
		else if (m_textPart->getPartCount() > 1)
		{
			msg->getHeader()->ContentType().setValue
				(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE));
		}
	}

	// Generate the attachments
	if (!m_attach.empty())
	{
		for (std::vector <attachment*>::const_iterator a = m_attach.begin() ;
		     a != m_attach.end() ; ++a)
		{
			(*a)->generateIn(*msg);
		}
	}

	// If there is only one part in the message, move it into the message
	// (hence, the message will not be multipart...)
	if (msg->getBody()->getPartCount() == 1)
	{
		const bodyPart& part = *msg->getBody()->getPartAt(0);

		// First, copy (and replace) the header fields
		const std::vector <const headerField*> fields = part.getHeader()->getFieldList();

		for (std::vector <const headerField*>::const_iterator it = fields.begin() ;
		     it != fields.end() ; ++it)
		{
			*(msg->getHeader()->getField((*it)->getName())) = **it;
		}

		// Second, copy the body contents and sub-parts (this also remove
		// the body part we are copying...)
		msg->getBody()->copyFrom(*part.getBody());
	}

	return (msg);
}


void messageBuilder::attach(attachment* attach)
{
	appendAttachment(attach);
}


void messageBuilder::appendAttachment(attachment* attach)
{
	m_attach.push_back(attach);
}


void messageBuilder::constructTextPart(const mediaType& type)
{
	textPart* part = NULL;

	try
	{
		part = textPartFactory::getInstance()->create(type);
	}
	catch (exceptions::no_factory_available& e)
	{
		throw;
	}

	delete (m_textPart);
	m_textPart = part;
}


textPart* messageBuilder::getTextPart()
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
	delete (m_attach[pos]);

	m_attach.erase(m_attach.begin() + pos);
}


const attachment* messageBuilder::getAttachmentAt(const int pos) const
{
	return (m_attach[pos]);
}


attachment* messageBuilder::getAttachmentAt(const int pos)
{
	return (m_attach[pos]);
}


const int messageBuilder::getAttachmentCount() const
{
	return (m_attach.size());
}


const std::vector <const attachment*> messageBuilder::getAttachmentList() const
{
	std::vector <const attachment*> res;

	res.reserve(m_attach.size());

	for (std::vector <attachment*>::const_iterator it = m_attach.begin() ;
	     it != m_attach.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


const std::vector <attachment*> messageBuilder::getAttachmentList()
{
	return (m_attach);
}


} // vmime
