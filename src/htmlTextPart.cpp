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

#include "vmime/htmlTextPart.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


htmlTextPart::~htmlTextPart()
{
	free_container(m_objects);
}


const mediaType htmlTextPart::getType() const
{
	return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML));
}


const int htmlTextPart::getPartCount() const
{
	return (m_plainText.isEmpty() ? 1 : 2);
}


void htmlTextPart::generateIn(bodyPart& /* message */, bodyPart& parent) const
{
	// Plain text
	if (!m_plainText.isEmpty())
	{
		// -- Create a new part
		bodyPart* part = new bodyPart();
		parent.getBody()->appendPart(part);

		// -- Set header fields
		part->getHeader()->ContentType().setValue(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
		part->getHeader()->ContentType().setCharset(m_charset);
		part->getHeader()->ContentTransferEncoding().setValue(encoding(encodingTypes::QUOTED_PRINTABLE));

		// -- Set contents
		part->getBody()->setContents(m_plainText);
	}

	// HTML text
	// -- Create a new part
	bodyPart* htmlPart = new bodyPart();

	// -- Set header fields
	htmlPart->getHeader()->ContentType().setValue(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML));
	htmlPart->getHeader()->ContentType().setCharset(m_charset);
	htmlPart->getHeader()->ContentTransferEncoding().setValue(encoding(encodingTypes::QUOTED_PRINTABLE));

	// -- Set contents
	htmlPart->getBody()->setContents(m_text);

	// Handle the case we have embedded objects
	if (!m_objects.empty())
	{
		// Create a "multipart/related" body part
		bodyPart* relPart = new bodyPart();
		parent.getBody()->appendPart(relPart);

		relPart->getHeader()->ContentType().
			setValue(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_RELATED));

		// Add the HTML part into this part
		relPart->getBody()->appendPart(htmlPart);

		// Also add objects into this part
		for (std::vector <embeddedObject*>::const_iterator it = m_objects.begin() ;
		     it != m_objects.end() ; ++it)
		{
			bodyPart* objPart = new bodyPart();
			relPart->getBody()->appendPart(objPart);

			string id = (*it)->getId();

			if (id.substr(0, 4) == "CID:")
				id = id.substr(4);

			objPart->getHeader()->ContentType().setValue((*it)->getType());
			objPart->getHeader()->ContentId().setValue(messageId("<" + id + ">"));
			objPart->getHeader()->ContentDisposition().setValue(contentDisposition(contentDispositionTypes::INLINE));
			objPart->getHeader()->ContentTransferEncoding().setValue((*it)->getEncoding());
				//encoding(encodingTypes::BASE64);

			objPart->getBody()->setContents((*it)->getData());
		}
	}
	else
	{
		// Add the HTML part into the parent part
		parent.getBody()->appendPart(htmlPart);
	}
}


void htmlTextPart::findEmbeddedParts(const bodyPart& part,
	std::vector <const bodyPart*>& cidParts, std::vector <const bodyPart*>& locParts)
{
	for (int i = 0 ; i < part.getBody()->getPartCount() ; ++i)
	{
		const bodyPart& p = *part.getBody()->getPartAt(i);

		try
		{
			dynamic_cast<messageIdField&>(*p.getHeader()->findField(fields::CONTENT_ID));
			cidParts.push_back(&p);
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-id" field.
			// Maybe there is a "Content-Location" field...
			try
			{
				dynamic_cast<messageIdField&>(*p.getHeader()->findField(fields::CONTENT_ID));
				locParts.push_back(&p);
			}
			catch (exceptions::no_such_field)
			{
				// No "Content-Location" field.
				// Cannot be an embedded object since it cannot be referenced in HTML text.
			}
		}

		findEmbeddedParts(p, cidParts, locParts);
	}
}


void htmlTextPart::addEmbeddedObject(const bodyPart& part, const string& id)
{
	mediaType type;

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*part.getHeader()->findField(fields::CONTENT_TYPE));

		type = ctf.getValue();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field: assume "application/octet-stream".
	}

	m_objects.push_back(new embeddedObject
		(part.getBody()->getContents(), part.getBody()->getEncoding(), id, type));
}


void htmlTextPart::parse(const bodyPart& message, const bodyPart& parent, const bodyPart& textPart)
{
	// Search for possible embedded objects in the _whole_ message.
	std::vector <const bodyPart*> cidParts;
	std::vector <const bodyPart*> locParts;

	findEmbeddedParts(message, cidParts, locParts);

	// Extract HTML text
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	textPart.getBody()->getContents().extract(adapter);

	const string data = oss.str();

	m_text = textPart.getBody()->getContents();

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*textPart.getHeader()->findField(fields::CONTENT_TYPE));

		m_charset = ctf.getCharset();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field.
	}
	catch (exceptions::no_such_parameter)
	{
		// No "charset" parameter.
	}

	// Extract embedded objects. The algorithm is quite simple: for each previously
	// found inline part, we check if its CID/Location is contained in the HTML text.
	for (std::vector <const bodyPart*>::const_iterator p = cidParts.begin() ; p != cidParts.end() ; ++p)
	{
		const messageIdField& midField = dynamic_cast<messageIdField&>
			(*(*p)->getHeader()->findField(fields::CONTENT_ID));

		const string searchFor("CID:" + midField.getValue().getId());

		if (data.find(searchFor) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, "CID:" + midField.getValue().getId());
		}
	}

	for (std::vector <const bodyPart*>::const_iterator p = locParts.begin() ; p != locParts.end() ; ++p)
	{
		const defaultField& locField = dynamic_cast<defaultField&>
			(*(*p)->getHeader()->findField(fields::CONTENT_LOCATION));

		if (data.find(locField.getValue()) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, locField.getValue());
		}
	}

	// Extract plain text, if any.
	findPlainTextPart(message, parent, textPart);
}


bool htmlTextPart::findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart)
{
	// We search for the nearest "multipart/alternative" part.
	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*part.getHeader()->findField(fields::CONTENT_TYPE));

		if (ctf.getValue().getType() == mediaTypes::MULTIPART &&
		    ctf.getValue().getSubType() == mediaTypes::MULTIPART_ALTERNATIVE)
		{
			bodyPart const* foundPart = NULL;

			for (int i = 0 ; i < part.getBody()->getPartCount() ; ++i)
			{
				const bodyPart* p = part.getBody()->getPartAt(i);

				if (p == &parent ||     // if "text/html" is in "multipart/related"
				    p == &textPart)     // if not...
				{
					foundPart = &(*p);
				}
			}

			if (foundPart)
			{
				bool found = false;

				// Now, search for the alternative plain text part
				for (int i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
				{
					const bodyPart& p = *part.getBody()->getPartAt(i);

					try
					{
						const contentTypeField& ctf = dynamic_cast<contentTypeField&>
							(*p.getHeader()->findField(fields::CONTENT_TYPE));

						if (ctf.getValue().getType() == mediaTypes::TEXT &&
						    ctf.getValue().getSubType() == mediaTypes::TEXT_PLAIN)
						{
							m_plainText = p.getBody()->getContents();
							found = true;
						}
					}
					catch (exceptions::no_such_field)
					{
						// No "Content-type" field.
					}
				}

				// If we don't have found the plain text part here, it means that
				// it does not exists (the MUA which built this message probably
				// did not include it...).
				return (found);
			}
		}
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field.
	}

	bool found = false;

	for (int i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
	{
		found = findPlainTextPart(*part.getBody()->getPartAt(i), parent, textPart);
	}

	return (found);
}


const charset& htmlTextPart::getCharset() const
{
	return (m_charset);
}


void htmlTextPart::setCharset(const charset& ch)
{
	m_charset = ch;
}


const contentHandler& htmlTextPart::getPlainText() const
{
	return (m_plainText);
}


void htmlTextPart::setPlainText(const contentHandler& plainText)
{
	m_plainText = plainText;
}


const contentHandler& htmlTextPart::getText() const
{
	return (m_text);
}


void htmlTextPart::setText(const contentHandler& text)
{
	m_text = text;
}


const int htmlTextPart::getObjectCount() const
{
	return (m_objects.size());
}


const htmlTextPart::embeddedObject* htmlTextPart::getObjectAt(const int pos) const
{
	return (m_objects[pos]);
}


const htmlTextPart::embeddedObject* htmlTextPart::findObject(const string& id) const
{
	for (std::vector <embeddedObject*>::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->getId() == id)
			return (*o);
	}

	throw exceptions::no_object_found();
}


const bool htmlTextPart::hasObject(const string& id) const
{
	for (std::vector <embeddedObject*>::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->getId() == id)
			return (true);
	}

	return (false);
}


const string htmlTextPart::addObject(const contentHandler& data,
	const vmime::encoding& enc, const mediaType& type)
{
	const messageId mid(messageId::generateId());
	const string id = "CID:" + mid.getId();

	m_objects.push_back(new embeddedObject(data, enc, id, type));

	return (id);
}


const string htmlTextPart::addObject(const contentHandler& data, const mediaType& type)
{
	return (addObject(data, encoding::decide(data), type));
}


const string htmlTextPart::addObject(const string& data, const mediaType& type)
{
	return (addObject(contentHandler(data), encoding::decide(data), type));
}



//
// htmlTextPart::embeddedObject
//

htmlTextPart::embeddedObject::embeddedObject
	(const contentHandler& data, const encoding& enc,
	 const string& id, const mediaType& type)
	: m_data(data), m_encoding(enc), m_id(id), m_type(type)
{
}


const contentHandler& htmlTextPart::embeddedObject::getData() const
{
	return (m_data);
}


const vmime::encoding& htmlTextPart::embeddedObject::getEncoding() const
{
	return (m_encoding);
}


const string& htmlTextPart::embeddedObject::getId() const
{
	return (m_id);
}


const mediaType& htmlTextPart::embeddedObject::getType() const
{
	return (m_type);
}


} // vmime
