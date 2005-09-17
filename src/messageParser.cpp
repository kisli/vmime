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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/messageParser.hpp"

#include "vmime/defaultAttachment.hpp"
#include "vmime/textPartFactory.hpp"


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
}


void messageParser::parse(const message& msg)
{
	// Header fields (if field is present, copy its value, else do nothing)
#ifndef VMIME_BUILDING_DOC

#define TRY_FIELD(var, type, name) \
	try { var = dynamic_cast<type&>(*msg.getHeader()->findField(name)).getValue(); } \
	catch (exceptions::no_such_field) { }

	TRY_FIELD(m_from, mailboxField, fields::FROM);

	TRY_FIELD(m_to, addressListField, fields::TO);
	TRY_FIELD(m_cc, addressListField, fields::CC);
	TRY_FIELD(m_bcc, addressListField, fields::BCC);

	TRY_FIELD(m_subject, textField, fields::SUBJECT);

#undef TRY_FIELD

#endif // VMIME_BUILDING_DOC

	// Date
	try
	{
		vmime::relayField& recv = dynamic_cast <vmime::relayField&>
			(*msg.getHeader()->findField(fields::RECEIVED));

		m_date = recv.getValue().getDate();
	}
	catch (vmime::exceptions::no_such_field&)
	{
		try
		{
			vmime::dateField& date = dynamic_cast <vmime::dateField&>
				(*msg.getHeader()->findField(fields::DATE));

			m_date = date.getValue();
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
	for (int i = 0 ; i < part.getBody()->getPartCount() ; ++i)
	{
		const bodyPart& p = *part.getBody()->getPartAt(i);
		const header& hdr = *p.getHeader();
		const body& bdy = *p.getBody();

		// Is this part an attachment?
		bool isAttachment = false;
		const contentDispositionField* contentDispField = NULL;

		try
		{
			const contentDispositionField& cdf = dynamic_cast<contentDispositionField&>
				(*hdr.findField(fields::CONTENT_DISPOSITION));

			if (cdf.getValue().getName() != contentDispositionTypes::INLINE)
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
					(*hdr.findField(fields::CONTENT_TYPE));

				type = ctf.getValue();
			}
			catch (exceptions::no_such_field)
			{
				// No "Content-type" field: assume "application/octet-stream".
				type = mediaType(mediaTypes::APPLICATION,
				                 mediaTypes::APPLICATION_OCTET_STREAM);
			}

			if (type.getType() != mediaTypes::TEXT &&
			    type.getType() != mediaTypes::MULTIPART)
			{
				isAttachment = true;
			}
		}

		if (isAttachment)
		{
			// Determine the media type of this attachment
			const contentTypeField* contTypeField = NULL;
			mediaType type;

			try
			{
				const contentTypeField& ctf = dynamic_cast<contentTypeField&>
					(*hdr.findField(fields::CONTENT_TYPE));

				type = ctf.getValue();

				contTypeField = &ctf;
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
					(*hdr.findField(fields::CONTENT_DESCRIPTION));

				description = cd.getValue();
			}
			catch (exceptions::no_such_field)
			{
				// No description available.
			}

			// Get the name/filename (if available)
			word name;

			// -- try the 'filename' parameter of 'Content-Disposition' field
			if (contentDispField != NULL)
			{
				try
				{
					name = contentDispField->getFilename();
				}
				catch (exceptions::no_such_parameter)
				{
					// No 'filename' parameter
				}
			}

			// -- try the 'name' parameter of 'Content-Type' field
			if (name.getBuffer().empty() && contTypeField != NULL)
			{
				try
				{
					ref <defaultParameter> prm = contTypeField->
						findParameter("name").dynamicCast <defaultParameter>();

					if (prm != NULL)
						name = prm->getValue();
				}
				catch (exceptions::no_such_parameter)
				{
					// No attachment name available.
				}
			}

			// Construct the attachment object
			ref <attachment> attach = vmime::create <defaultAttachment>
				(bdy.getContents()->clone().dynamicCast <contentHandler>(),
				 bdy.getEncoding(), type, description, name);

			if (contentDispField != NULL)
			{
				m_attachInfo.insert(std::map <attachment*, ref <contentDispositionField> >::
					value_type(attach.get(), contentDispField->clone().
						dynamicCast <contentDispositionField>()));
			}

			// Add the attachment to the list
			m_attach.push_back(attach);
		}

		// Try to find attachments in sub-parts
		if (bdy.getPartCount())
			findAttachments(p);
	}
}


void messageParser::findTextParts(const bodyPart& msg, const bodyPart& part)
{
	// Handle the case in which the message is not multipart: if the body part is
	// "text/*", take this part.
	if (part.getBody()->getPartCount() == 0)
	{
		mediaType type(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN);
		bool accept = false;

		try
		{
			const contentTypeField& ctf = dynamic_cast<contentTypeField&>
				(*msg.getHeader()->findField(fields::CONTENT_TYPE));

			if (ctf.getValue().getType() == mediaTypes::TEXT)
			{
				type = ctf.getValue();
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
			ref <textPart> txtPart = textPartFactory::getInstance()->create(type);
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


bool messageParser::findSubTextParts(const bodyPart& msg, const bodyPart& part)
{
	// In general, all the text parts are contained in parallel in the same
	// parent part (or message).
	// So, wherever the text parts are, all we have to do is to find the first
	// MIME part which is a text part.

	std::vector <ref <const bodyPart> > textParts;

	for (int i = 0 ; i < part.getBody()->getPartCount() ; ++i)
	{
		const ref <const bodyPart> p = part.getBody()->getPartAt(i);

		try
		{
			const contentTypeField& ctf = dynamic_cast <const contentTypeField&>
				(*(p->getHeader()->findField(fields::CONTENT_TYPE)));

			if (ctf.getValue().getType() == mediaTypes::TEXT)
			{
				textParts.push_back(p);
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
		for (std::vector <ref <const bodyPart> >::const_iterator p = textParts.begin() ;
		     p != textParts.end() ; ++p)
		{
			const contentTypeField& ctf = dynamic_cast <const contentTypeField&>
				(*((*p)->getHeader()->findField(fields::CONTENT_TYPE)));

			try
			{
				ref <textPart> txtPart = textPartFactory::getInstance()->create(ctf.getValue());
				txtPart->parse(msg, part, **p);

				m_textParts.push_back(txtPart);
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

		for (int i = 0 ; !found && (i < part.getBody()->getPartCount()) ; ++i)
		{
			found = findSubTextParts(msg, *part.getBody()->getPartAt(i));
		}

		return found;
	}
}


const ref <const contentDispositionField> messageParser::getAttachmentInfo(const ref <const attachment> a) const
{
	std::map <attachment*, ref <contentDispositionField> >::const_iterator
		it = m_attachInfo.find(ref <attachment>(a.constCast <attachment>()).get());

	return (it != m_attachInfo.end() ? (*it).second : NULL);
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


const std::vector <ref <const attachment> > messageParser::getAttachmentList() const
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


const int messageParser::getAttachmentCount() const
{
	return (m_attach.size());
}


const ref <const attachment> messageParser::getAttachmentAt(const int pos) const
{
	return (m_attach[pos]);
}


const std::vector <ref <const textPart> > messageParser::getTextPartList() const
{
	std::vector <ref <const textPart> > res;

	res.reserve(m_textParts.size());

	for (std::vector <ref <textPart> >::const_iterator it = m_textParts.begin() ;
	     it != m_textParts.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


const int messageParser::getTextPartCount() const
{
	return (m_textParts.size());
}


const ref <const textPart> messageParser::getTextPartAt(const int pos) const
{
	return (m_textParts[pos]);
}


} // vmime
