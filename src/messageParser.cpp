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

#include "messageParser.hpp"

#include "defaultAttachment.hpp"
#include "textPartFactory.hpp"

#include "relayField.hpp"


namespace vmime
{


messageParser::messageParser(const string& buffer)
{
	vmime::message msg;
	msg.parse(buffer);

	parse(msg);
}


messageParser::messageParser(const message& msg)
{
	parse(msg);
}


messageParser::~messageParser()
{
	free_container(m_attach);
	free_container(m_textParts);

	for (std::map <attachment*, contentDispositionField*>::iterator
	     it = m_attachInfo.begin() ; it != m_attachInfo.end() ; ++it)
	{
		delete ((*it).second);
	}
}


void messageParser::parse(const message& msg)
{
	// Header fields (if field is present, copy its value, else do nothing)
#define TRY_FIELD(x) try { x; } catch (exceptions::no_such_field) { }
	TRY_FIELD(m_from = dynamic_cast<mailboxField&>(msg.header().fields.find(headerField::From)).value());

	TRY_FIELD(m_to = dynamic_cast<addressListField&>(msg.header().fields.find(headerField::To)).value());
	TRY_FIELD(m_cc = dynamic_cast<addressListField&>(msg.header().fields.find(headerField::Cc)).value());
	TRY_FIELD(m_bcc = dynamic_cast<addressListField&>(msg.header().fields.find(headerField::Bcc)).value());

	TRY_FIELD(m_subject = dynamic_cast<textField&>(msg.header().fields.find(headerField::Subject)).value());
#undef TRY_FIELD

	// Date
	try
	{
		vmime::relayField& recv = static_cast<vmime::relayField&>(msg.header().fields.find(headerField::Received));
		m_date = recv.date();
	}
	catch (vmime::exceptions::no_such_field&)
	{
		try
		{
			vmime::dateField& date = static_cast<vmime::dateField&>(msg.header().fields.find(headerField::Date));
			m_date = date.value();
		}
		catch (vmime::exceptions::no_such_field&)
		{
			m_date = datetime::now();
		}
	}

	// Attachments
	findAttachments(msg);

	// Text parts
	findTextParts(msg, msg);
}


void messageParser::findAttachments(const bodyPart& part)
{
	// We simply search for parts that are not "Content-disposition: inline".
	for (body::const_iterator p = part.body().parts.begin() ; p != part.body().parts.end() ; ++p)
	{
		const header& hdr = (*p).header();
		const body& bdy = (*p).body();

		// Is this part an attachment?
		bool isAttachment = false;
		const contentDispositionField* contentDispField = NULL;

		try
		{
			const contentDispositionField& cdf = dynamic_cast<contentDispositionField&>
				(hdr.fields.find(headerField::ContentDisposition));

			if (cdf.value().name() != dispositionTypes::INLINE)
			{
				contentDispField = &cdf;
				isAttachment = true;
			}
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-disposition" field: assume "attachment" if
			// type is not "text/..." or "multipart/...".
			mediaType type;

			try
			{
				const contentTypeField& ctf = dynamic_cast<contentTypeField&>
					(hdr.fields.find(headerField::ContentType));

				type = ctf.value();
			}
			catch (exceptions::no_such_field)
			{
				// No "Content-type" field: assume "application/octet-stream".
				type = mediaType(mediaTypes::APPLICATION,
				                 mediaTypes::APPLICATION_OCTET_STREAM);
			}

			if (type.type() != mediaTypes::TEXT && type.type() != mediaTypes::MULTIPART)
				isAttachment = true;
		}

		if (isAttachment)
		{
			// Determine the media type of this attachment
			mediaType type;

			try
			{
				const contentTypeField& ctf = dynamic_cast<contentTypeField&>
					(hdr.fields.find(headerField::ContentType));

				type = ctf.value();
			}
			catch (exceptions::no_such_field)
			{
				// No "Content-type" field: assume "application/octet-stream".
				type = mediaType(mediaTypes::APPLICATION,
				                 mediaTypes::APPLICATION_OCTET_STREAM);
			}

			// Get the description (if available)
			text description;

			try
			{
				const textField& cd = dynamic_cast<textField&>
					(hdr.fields.find(headerField::ContentDescription));

				description = cd.value();
			}
			catch (exceptions::no_such_field)
			{
				// No description available.
			}

			// Construct the attachment object
			attachment* attach = new defaultAttachment
				(bdy.contents(), bdy.encoding(), type, description);

			if (contentDispField != NULL)
			{
				m_attachInfo.insert(std::map <attachment*, contentDispositionField*>::
					value_type(attach, static_cast <contentDispositionField*>
						(contentDispField->clone())));
			}

			// Add the attachment to the list
			m_attach.push_back(attach);
		}

		// Try to find attachments in sub-parts
		if (bdy.parts.size())
			findAttachments(*p);
	}
}


void messageParser::findTextParts(const bodyPart& msg, const bodyPart& part)
{
	// Handle the case in which the message is not multipart: if the body part is
	// "text/*", take this part.
	if (part.body().parts.count() == 0)
	{
		mediaType type(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN);
		bool accept = false;

		try
		{
			const contentTypeField& ctf = dynamic_cast<contentTypeField&>
				(msg.header().fields.find(headerField::ContentType));

			if (ctf.value().type() == mediaTypes::TEXT)
			{
				type = ctf.value();
				accept = true;
			}
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-type" field: assume "text/plain".
			accept = true;
		}

		if (accept)
		{
			textPart* textPart = textPartFactory::getInstance()->create(type);
			textPart->parse(msg, msg, msg);

			m_textParts.push_back(textPart);
		}
	}
	// Multipart message
	else
	{
		findSubTextParts(msg, part);
	}
}


bool messageParser::findSubTextParts(const bodyPart& msg, const bodyPart& part)
{
	// In general, all the text parts are contained in parallel in the same
	// parent part (or message).
	// So, wherever the text parts are, all we have to do is to find the first
	// MIME part which is a text part.

	std::vector <const bodyPart*> textParts;

	for (body::const_iterator p = part.body().parts.begin() ;
	     p != part.body().parts.end() ; ++p)
	{
		try
		{
			const contentTypeField& ctf = dynamic_cast<contentTypeField&>
				((*p).header().fields.find(headerField::ContentType));

			if (ctf.value().type() == mediaTypes::TEXT)
			{
				textParts.push_back(&(*p));
			}
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-type" field.
		}
	}

	if (textParts.size())
	{
		// Okay. So we have found at least one text part
		for (std::vector <const bodyPart*>::const_iterator p = textParts.begin() ; p != textParts.end() ; ++p)
		{
			const contentTypeField& ctf = dynamic_cast<contentTypeField&>
				((*p)->header().fields.find(headerField::ContentType));

			try
			{
				textPart* textPart = textPartFactory::getInstance()->create(ctf.value());
				textPart->parse(msg, part, **p);

				m_textParts.push_back(textPart);
			}
			catch (exceptions::no_factory_available& e)
			{
				// Content-type not recognized.
			}
		}

		//return true;
	}

	//else
	{
		bool found = false;

		for (body::const_iterator p = part.body().parts.begin() ;
		     !found && p != part.body().parts.end() ; ++p)
		{
			found = findSubTextParts(msg, *p);
		}

		return found;
	}
}


const contentDispositionField* messageParser::attachmentInfo(attachment* a) const
{
	std::map <attachment*, contentDispositionField*>::const_iterator
		it = m_attachInfo.find(a);

	return (it != m_attachInfo.end() ? (*it).second : NULL);
}


} // vmime
