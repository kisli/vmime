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

#include "vmime/messageParser.hpp"

#include "vmime/attachmentHelper.hpp"

#include "vmime/defaultAttachment.hpp"
#include "vmime/textPartFactory.hpp"

#include "vmime/relay.hpp"
#include "vmime/contentTypeField.hpp"
#include "vmime/contentDispositionField.hpp"


namespace vmime
{


messageParser::messageParser(const string& buffer)
{
	shared_ptr <message> msg = make_shared <message>();
	msg->parse(buffer);

	parse(msg);
}


messageParser::messageParser(shared_ptr <const message> msg)
{
	parse(msg);
}


messageParser::~messageParser()
{
}


void messageParser::parse(shared_ptr <const message> msg)
{
	// Header fields (if field is present, copy its value, else do nothing)
#ifndef VMIME_BUILDING_DOC

#define TRY_FIELD(var, type, name) \
	{ \
		shared_ptr <type> fldValue = msg->getHeader()->findFieldValue <type>(name); \
		if (fldValue) \
			var = *fldValue; \
	}

	TRY_FIELD(m_from, mailbox, fields::FROM);

	TRY_FIELD(m_to, addressList, fields::TO);
	TRY_FIELD(m_cc, addressList, fields::CC);
	TRY_FIELD(m_bcc, addressList, fields::BCC);

	TRY_FIELD(m_subject, text, fields::SUBJECT);

#undef TRY_FIELD

#endif // VMIME_BUILDING_DOC

	// Date
	shared_ptr <const headerField> recv = msg->getHeader()->findField(fields::RECEIVED);

	if (recv)
	{
		m_date = recv->getValue <relay>()->getDate();
	}
	else
	{
		shared_ptr <const headerField> date = msg->getHeader()->findField(fields::DATE);

		if (date)
			m_date = *date->getValue <datetime>();
		else
			m_date = datetime::now();
	}

	// Attachments
	findAttachments(msg);

	// Text parts
	findTextParts(msg, msg);
}


void messageParser::findAttachments(shared_ptr <const message> msg)
{
	m_attach = attachmentHelper::findAttachmentsInMessage(msg);
}


void messageParser::findTextParts(shared_ptr <const bodyPart> msg, shared_ptr <const bodyPart> part)
{
	// Handle the case in which the message is not multipart: if the body part is
	// "text/*", take this part.
	if (part->getBody()->getPartCount() == 0)
	{
		mediaType type(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN);
		bool accept = false;

		shared_ptr <const contentTypeField> ctf =
			msg->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

		if (ctf)
		{
			const mediaType ctfType = *ctf->getValue <mediaType>();

			if (ctfType.getType() == mediaTypes::TEXT)
			{
				type = ctfType;
				accept = true;
			}
		}
		else
		{
			// No "Content-type" field: assume "text/plain".
			accept = true;
		}

		if (accept)
		{
			shared_ptr <textPart> txtPart = textPartFactory::getInstance()->create(type);
			txtPart->parse(msg, msg, msg);

			m_textParts.push_back(txtPart);
		}
	}
	// Multipart message
	else
	{
		findSubTextParts(msg, part);
	}
}


bool messageParser::findSubTextParts(shared_ptr <const bodyPart> msg, shared_ptr <const bodyPart> part)
{
	// In general, all the text parts are contained in parallel in the same
	// parent part (or message).
	// So, wherever the text parts are, all we have to do is to find the first
	// MIME part which is a text part.

	std::vector <shared_ptr <const bodyPart> > textParts;

	for (size_t i = 0 ; i < part->getBody()->getPartCount() ; ++i)
	{
		const shared_ptr <const bodyPart> p = part->getBody()->getPartAt(i);

		shared_ptr <const contentTypeField> ctf =
			p->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

		if (ctf)
		{
			const mediaType type = *ctf->getValue <mediaType>();
			contentDisposition disp; // default should be inline

			if (type.getType() == mediaTypes::TEXT)
			{
				shared_ptr <const contentDispositionField> cdf = p->getHeader()->
					findField <contentDispositionField>(fields::CONTENT_DISPOSITION);

				if (cdf)
				{
					disp = *cdf->getValue <contentDisposition>();
				}
				else
				{
					// No "Content-Disposition" field, assume default
				}

				if (disp.getName() == contentDispositionTypes::INLINE)
					textParts.push_back(p);
			}
		}
		else
		{
			// No "Content-type" field.
		}
	}

	if (textParts.size())
	{
		// Okay. So we have found at least one text part
		for (std::vector <shared_ptr <const bodyPart> >::const_iterator p = textParts.begin() ;
		     p != textParts.end() ; ++p)
		{
			const contentTypeField& ctf =
				*(*p)->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

			const mediaType type = *ctf.getValue <mediaType>();

			try
			{
				shared_ptr <textPart> txtPart = textPartFactory::getInstance()->create(type);
				txtPart->parse(msg, part, *p);

				m_textParts.push_back(txtPart);
			}
			catch (exceptions::no_factory_available& e)
			{
				// Content-type not recognized.
			}
		}
	}

	bool found = false;

	for (size_t i = 0 ; !found && (i < part->getBody()->getPartCount()) ; ++i)
	{
		found = findSubTextParts(msg, part->getBody()->getPartAt(i));
	}

	return found;
}


const mailbox& messageParser::getExpeditor() const
{
	return (m_from);
}


const addressList& messageParser::getRecipients() const
{
	return (m_to);
}


const addressList& messageParser::getCopyRecipients() const
{
	return (m_cc);
}


const addressList& messageParser::getBlindCopyRecipients() const
{
	return (m_bcc);
}


const text& messageParser::getSubject() const
{
	return (m_subject);
}


const datetime& messageParser::getDate() const
{
	return (m_date);
}


const std::vector <shared_ptr <const attachment> > messageParser::getAttachmentList() const
{
	return m_attach;
}


size_t messageParser::getAttachmentCount() const
{
	return (m_attach.size());
}


const shared_ptr <const attachment> messageParser::getAttachmentAt(const size_t pos) const
{
	return (m_attach[pos]);
}


const std::vector <shared_ptr <const textPart> > messageParser::getTextPartList() const
{
	std::vector <shared_ptr <const textPart> > res;

	res.reserve(m_textParts.size());

	for (std::vector <shared_ptr <textPart> >::const_iterator it = m_textParts.begin() ;
	     it != m_textParts.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


size_t messageParser::getTextPartCount() const
{
	return (m_textParts.size());
}


const shared_ptr <const textPart> messageParser::getTextPartAt(const size_t pos) const
{
	return (m_textParts[pos]);
}


} // vmime
