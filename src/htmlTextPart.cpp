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

#include "htmlTextPart.hpp"
#include "exception.hpp"


namespace vmime
{


htmlTextPart::~htmlTextPart()
{
}


const mediaType htmlTextPart::type() const
{
	return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML));
}


const int htmlTextPart::getPartCount() const
{
	return (m_plainText.empty() ? 1 : 2);
}


void htmlTextPart::generateIn(bodyPart& /* message */, bodyPart& parent) const
{
	// Plain text
	if (!m_plainText.empty())
	{
		// -- Create a new part
		bodyPart* part = new bodyPart();
		parent.body().parts.append(part);

		// -- Set header fields
		part->header().fields.ContentType() = mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN);
		part->header().fields.ContentType().charset() = m_charset;
		part->header().fields.ContentTransferEncoding() = encoding(encodingTypes::QUOTED_PRINTABLE);

		// -- Set contents
		part->body().contents() = m_plainText;
	}

	// HTML text
	// -- Create a new part
	bodyPart* htmlPart = new bodyPart();

	// -- Set header fields
	htmlPart->header().fields.ContentType() = mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML);
	htmlPart->header().fields.ContentType().charset() = m_charset;
	htmlPart->header().fields.ContentTransferEncoding() = encoding(encodingTypes::QUOTED_PRINTABLE);

	// -- Set contents
	htmlPart->body().contents() = m_text;

	// Handle the case we have embedded objects
	if (!embeddedObjects.empty())
	{
		// Create a "multipart/related" body part
		bodyPart* relPart = new bodyPart();
		parent.body().parts.append(relPart);

		relPart->header().fields.ContentType() = mediaType
			(mediaTypes::MULTIPART, mediaTypes::MULTIPART_RELATED);

		// Add the HTML part into this part
		relPart->body().parts.append(htmlPart);

		// Also add images into this part
		for (embeddedObjectsContainer::const_iterator i = embeddedObjects.begin() ;
		     i != embeddedObjects.end() ; ++i)
		{
			bodyPart* objPart = new bodyPart();
			relPart->body().parts.append(objPart);

			string id = (*i).id();

			if (id.substr(0, 4) == "CID:")
				id = id.substr(4);

			objPart->header().fields.ContentType() = (*i).type();
			objPart->header().fields.ContentId() = messageId("<" + id + ">");
			objPart->header().fields.ContentDisposition() = disposition(dispositionTypes::INLINE);
			objPart->header().fields.ContentTransferEncoding() = (*i).encoding();
				//encoding(encodingTypes::BASE64);

			objPart->body().contents() = (*i).data();
		}
	}
	else
	{
		// Add the HTML part into the parent part
		parent.body().parts.append(htmlPart);
	}
}


void htmlTextPart::findEmbeddedParts(const bodyPart& part,
	std::vector <const bodyPart*>& cidParts, std::vector <const bodyPart*>& locParts)
{
	for (body::const_iterator p = part.body().parts.begin() ; p != part.body().parts.end() ; ++p)
	{
		try
		{
			dynamic_cast<messageIdField&>((*p).header().fields.find(headerField::ContentId));
			cidParts.push_back(&(*p));
		}
		catch (exceptions::no_such_field)
		{
			// No "Content-id" field.
			// Maybe there is a "Content-Location" field...
			try
			{
				dynamic_cast<messageIdField&>((*p).header().fields.find(headerField::ContentId));
				locParts.push_back(&(*p));
			}
			catch (exceptions::no_such_field)
			{
				// No "Content-Location" field.
				// Cannot be an embedded object since it cannot be referenced in HTML text.
			}
		}

		findEmbeddedParts((*p), cidParts, locParts);
	}
}


void htmlTextPart::addEmbeddedObject(const bodyPart& part, const string& id)
{
	mediaType type;

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(part.header().fields.find(headerField::ContentType));

		type = ctf.value();
	}
	catch (exceptions::no_such_field)
	{
		// No "Content-type" field: assume "application/octet-stream".
	}

	embeddedObjects.m_list.push_back(new embeddedObject
		(part.body().contents(), part.body().encoding(), id, type));
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

	textPart.body().contents().extract(adapter);

	const string data = oss.str();

	m_text = textPart.body().contents();

	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(textPart.header().fields.find(headerField::ContentType));

		m_charset = ctf.charset();
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
			((**p).header().fields.find(headerField::ContentId));

		const string searchFor("CID:" + midField.value().id());

		if (data.find(searchFor) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, "CID:" + midField.value().id());
		}
	}

	for (std::vector <const bodyPart*>::const_iterator p = locParts.begin() ; p != locParts.end() ; ++p)
	{
		const defaultField& locField = dynamic_cast<defaultField&>
			((**p).header().fields.find(headerField::ContentLocation));

		if (data.find(locField.value()) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, locField.value());
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
			(part.header().fields.find(headerField::ContentType));

		if (ctf.value().type() == mediaTypes::MULTIPART &&
		    ctf.value().subType() == mediaTypes::MULTIPART_ALTERNATIVE)
		{
			bodyPart const* foundPart = NULL;

			for (body::const_iterator p = part.body().parts.begin() ; !foundPart && p != part.body().parts.end() ; ++p)
			{
				if (&(*p) == &parent ||     // if "text/html" is in "multipart/related"
				    &(*p) == &textPart)     // if not...
				{
					foundPart = &(*p);
				}
			}

			if (foundPart)
			{
				bool found = false;

				// Now, search for the alternative plain text part
				for (body::const_iterator p = part.body().parts.begin() ;
				     !found && p != part.body().parts.end() ; ++p)
				{
					try
					{
						const contentTypeField& ctf = dynamic_cast<contentTypeField&>
							((*p).header().fields.find(headerField::ContentType));

						if (ctf.value().type() == mediaTypes::TEXT &&
						    ctf.value().subType() == mediaTypes::TEXT_PLAIN)
						{
							m_plainText = (*p).body().contents();
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

	for (body::const_iterator p = part.body().parts.begin() ; !found && p != part.body().parts.end() ; ++p)
	{
		found = findPlainTextPart(*p, parent, textPart);
	}

	return (found);
}



////////////////////////////////
// Embedded objects container //
////////////////////////////////


htmlTextPart::embeddedObjectsContainer::~embeddedObjectsContainer()
{
	free_container(m_list);
}


const htmlTextPart::embeddedObject& htmlTextPart::embeddedObjectsContainer::find(const string& id) const
{
	for (std::vector <embeddedObject*>::const_iterator o = m_list.begin() ; o != m_list.end() ; ++o)
	{
		if ((**o).id() == id)
			return (**o);
	}

	throw exceptions::no_object_found();
}


const bool htmlTextPart::embeddedObjectsContainer::has(const string& id) const
{
	for (std::vector <embeddedObject*>::const_iterator o = m_list.begin() ; o != m_list.end() ; ++o)
	{
		if ((**o).id() == id)
			return (true);
	}

	return (false);
}


const string htmlTextPart::embeddedObjectsContainer::add
	(const contentHandler& data, const vmime::encoding& enc, const mediaType& type)
{
	const messageId mid(messageId::generateId());
	const string id = "CID:" + mid.id();

	m_list.push_back(new embeddedObject(data, enc, id, type));

	return (id);
}


const string htmlTextPart::embeddedObjectsContainer::add
	(const contentHandler& data, const mediaType& type)
{
	return (add(data, encoding::decide(data), type));
}


const string htmlTextPart::embeddedObjectsContainer::add
	(const string& data, const mediaType& type)
{
	return (add(contentHandler(data), encoding::decide(data), type));
}


} // vmime
