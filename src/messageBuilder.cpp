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

#include "messageBuilder.hpp"

#include "textPartFactory.hpp"


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
	msg->header().fields.Subject() = m_subject;

	if (m_from.empty())
		throw exceptions::no_expeditor();

	if (m_to.empty() || (*m_to.begin()).empty())
		throw exceptions::no_recipient();

	msg->header().fields.From() = m_from;
	msg->header().fields.To() = m_to;

	if (!m_cc.empty())
		msg->header().fields.Cc() = m_cc;

	if (!m_bcc.empty())
		msg->header().fields.Bcc() = m_bcc;

	// Add a "Date" field
	msg->header().fields.Date() = datetime::now();

	// Add a "Mime-Version" header field
	msg->header().fields.MimeVersion().value() = MIME_VERSION;

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
		msg->header().fields.ContentType() = mediaType
			(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED);

		// Create a sub-part "multipart/alternative" for text parts
		bodyPart* subPart = new bodyPart;
		msg->body().parts.append(subPart);

		subPart->header().fields.ContentType() = mediaType
			(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE);

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
			msg->header().fields.ContentType() = mediaType
				(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED);
		}
		// Else, set it to "multipart/alternative" if there are more than one text part.
		else if (m_textPart->getPartCount() > 1)
		{
			msg->header().fields.ContentType() = mediaType
				(mediaTypes::MULTIPART, mediaTypes::MULTIPART_ALTERNATIVE);
		}
	}

	// Generate the attachments
	if (!m_attach.empty())
	{
		for (std::vector <attachment*>::const_iterator a = m_attach.begin() ; a != m_attach.end() ; ++a)
		{
			(*a)->generateIn(*msg);
		}
	}

	// If there is only one part in the message, move it into the message
	// (hence, the message will not be multipart...)
	if (msg->body().parts.size() == 1)
	{
		const bodyPart& part = msg->body().parts.front();

		// First, copy (and replace) the header fields
		const header::fieldsContainer& hdr = part.header().fields;

		for (header::const_iterator f = hdr.begin() ; f != hdr.end() ; ++f)
			msg->header().fields.get((*f).name()) = *f;

		// Second, copy the body contents and sub-parts (this also remove
		// the body part we are copying...)
		msg->body() = part.body();
	}

	return (msg);
}


void messageBuilder::attach(attachment* attach)
{
	m_attach.push_back(attach);
}


void messageBuilder::constructTextPart(const mediaType& type)
{
	class textPart* part = NULL;

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


class textPart& messageBuilder::textPart()
{
	return (*m_textPart);
}


} // vmime
