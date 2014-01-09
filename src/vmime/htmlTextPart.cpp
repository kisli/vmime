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

#include "vmime/htmlTextPart.hpp"
#include "vmime/exception.hpp"

#include "vmime/contentTypeField.hpp"
#include "vmime/contentDisposition.hpp"
#include "vmime/text.hpp"

#include "vmime/emptyContentHandler.hpp"
#include "vmime/stringContentHandler.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime
{


htmlTextPart::htmlTextPart()
	: m_plainText(make_shared <emptyContentHandler>()),
	  m_text(make_shared <emptyContentHandler>())
{
}


htmlTextPart::~htmlTextPart()
{
}


const mediaType htmlTextPart::getType() const
{
	return mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML);
}


size_t htmlTextPart::getPartCount() const
{
	return (m_plainText->isEmpty() ? 1 : 2);
}


void htmlTextPart::generateIn(shared_ptr <bodyPart> /* message */, shared_ptr <bodyPart> parent) const
{
	// Plain text
	if (!m_plainText->isEmpty())
	{
		// -- Create a new part
		shared_ptr <bodyPart> part = make_shared <bodyPart>();
		parent->getBody()->appendPart(part);

		// -- Set contents
		part->getBody()->setContents(m_plainText,
			mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN), m_charset,
			encoding::decide(m_plainText, m_charset, encoding::USAGE_TEXT));
	}

	// HTML text
	// -- Create a new part
	shared_ptr <bodyPart> htmlPart = make_shared <bodyPart>();

	// -- Set contents
	htmlPart->getBody()->setContents(m_text,
		mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML), m_charset,
		encoding::decide(m_text, m_charset, encoding::USAGE_TEXT));

	// Handle the case we have embedded objects
	if (!m_objects.empty())
	{
		// Create a "multipart/related" body part
		shared_ptr <bodyPart> relPart = make_shared <bodyPart>();
		parent->getBody()->appendPart(relPart);

		relPart->getHeader()->ContentType()->
			setValue(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_RELATED));

		// Add the HTML part into this part
		relPart->getBody()->appendPart(htmlPart);

		// Also add objects into this part
		for (std::vector <shared_ptr <embeddedObject> >::const_iterator it = m_objects.begin() ;
		     it != m_objects.end() ; ++it)
		{
			shared_ptr <bodyPart> objPart = make_shared <bodyPart>();
			relPart->getBody()->appendPart(objPart);

			string id = (*it)->getId();

			if (id.length() >= 4 &&
			    (id[0] == 'c' || id[0] == 'C') &&
			    (id[1] == 'i' || id[1] == 'I') &&
			    (id[2] == 'd' || id[2] == 'D') &&
			    id[3] == ':')
			{
				id = id.substr(4);
			}

			objPart->getHeader()->ContentType()->setValue((*it)->getType());
			objPart->getHeader()->ContentId()->setValue(messageId("<" + id + ">"));
			objPart->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::INLINE));
			objPart->getHeader()->ContentTransferEncoding()->setValue((*it)->getEncoding());
				//encoding(encodingTypes::BASE64);

			objPart->getBody()->setContents((*it)->getData()->clone());
		}
	}
	else
	{
		// Add the HTML part into the parent part
		parent->getBody()->appendPart(htmlPart);
	}
}


void htmlTextPart::findEmbeddedParts(const bodyPart& part,
	std::vector <shared_ptr <const bodyPart> >& cidParts, std::vector <shared_ptr <const bodyPart> >& locParts)
{
	for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i)
	{
		shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

		// For a part to be an embedded object, it must have either a
		// Content-Id field or a Content-Location field.
		if (p->getHeader()->hasField(fields::CONTENT_ID))
			cidParts.push_back(p);

		if (p->getHeader()->hasField(fields::CONTENT_LOCATION))
			locParts.push_back(p);

		findEmbeddedParts(*p, cidParts, locParts);
	}
}


void htmlTextPart::addEmbeddedObject(const bodyPart& part, const string& id,
	const embeddedObject::ReferenceType refType)
{
	// The object may already exists. This can happen if an object is
	// identified by both a Content-Id and a Content-Location. In this
	// case, there will be two embedded objects with two different IDs
	// but referencing the same content.

	mediaType type;

	shared_ptr <const headerField> ctf =
		part.getHeader()->findField(fields::CONTENT_TYPE);

	if (ctf)
	{
		type = *ctf->getValue <mediaType>();
	}
	else
	{
		// No "Content-type" field: assume "application/octet-stream".
	}

	m_objects.push_back(make_shared <embeddedObject>
		(vmime::clone(part.getBody()->getContents()),
		 part.getBody()->getEncoding(), id, type, refType));
}


void htmlTextPart::parse(shared_ptr <const bodyPart> message, shared_ptr <const bodyPart> parent, shared_ptr <const bodyPart> textPart)
{
	// Search for possible embedded objects in the _whole_ message.
	std::vector <shared_ptr <const bodyPart> > cidParts;
	std::vector <shared_ptr <const bodyPart> > locParts;

	findEmbeddedParts(*message, cidParts, locParts);

	// Extract HTML text
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	textPart->getBody()->getContents()->extract(adapter);

	const string data = oss.str();

	m_text = textPart->getBody()->getContents()->clone();

	// Find charset
	shared_ptr <const contentTypeField> ctf =
		textPart->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf && ctf->hasCharset())
		m_charset = ctf->getCharset();
	else
		m_charset = charset();

	// Extract embedded objects. The algorithm is quite simple: for each previously
	// found inline part, we check if its CID/Location is contained in the HTML text.
	for (std::vector <shared_ptr <const bodyPart> >::const_iterator p = cidParts.begin() ; p != cidParts.end() ; ++p)
	{
		const shared_ptr <const headerField> midField =
			(*p)->getHeader()->findField(fields::CONTENT_ID);

		const messageId mid = *midField->getValue <messageId>();

		if (data.find("CID:" + mid.getId()) != string::npos ||
		    data.find("cid:" + mid.getId()) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, mid.getId(), embeddedObject::REFERENCED_BY_ID);
		}
	}

	for (std::vector <shared_ptr <const bodyPart> >::const_iterator p = locParts.begin() ; p != locParts.end() ; ++p)
	{
		const shared_ptr <const headerField> locField =
			(*p)->getHeader()->findField(fields::CONTENT_LOCATION);

		const text loc = *locField->getValue <text>();
		const string locStr = loc.getWholeBuffer();

		if (data.find(locStr) != string::npos)
		{
			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			addEmbeddedObject(**p, locStr, embeddedObject::REFERENCED_BY_LOCATION);
		}
	}

	// Extract plain text, if any.
	if (!findPlainTextPart(*message, *parent, *textPart))
	{
		m_plainText = make_shared <emptyContentHandler>();
	}
}


bool htmlTextPart::findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart)
{
	// We search for the nearest "multipart/alternative" part.
	const shared_ptr <const headerField> ctf =
		part.getHeader()->findField(fields::CONTENT_TYPE);

	if (ctf)
	{
		const mediaType type = *ctf->getValue <mediaType>();

		if (type.getType() == mediaTypes::MULTIPART &&
		    type.getSubType() == mediaTypes::MULTIPART_ALTERNATIVE)
		{
			shared_ptr <const bodyPart> foundPart;

			for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i)
			{
				const shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

				if (p.get() == &parent ||     // if "text/html" is in "multipart/related"
				    p.get() == &textPart)     // if not...
				{
					foundPart = p;
				}
			}

			if (foundPart)
			{
				bool found = false;

				// Now, search for the alternative plain text part
				for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
				{
					const shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

					const shared_ptr <const headerField> ctf =
						p->getHeader()->findField(fields::CONTENT_TYPE);

					if (ctf)
					{

						const mediaType type = *ctf->getValue <mediaType>();

						if (type.getType() == mediaTypes::TEXT &&
						    type.getSubType() == mediaTypes::TEXT_PLAIN)
						{
							m_plainText = p->getBody()->getContents()->clone();
							found = true;
						}
					}
					else
					{
						// No "Content-type" field.
					}
				}

				// If we don't have found the plain text part here, it means that
				// it does not exists (the MUA which built this message probably
				// did not include it...).
				return found;
			}
		}
	}
	else
	{
		// No "Content-type" field.
	}

	bool found = false;

	for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i)
	{
		found = findPlainTextPart(*part.getBody()->getPartAt(i), parent, textPart);
	}

	return found;
}


const charset& htmlTextPart::getCharset() const
{
	return m_charset;
}


void htmlTextPart::setCharset(const charset& ch)
{
	m_charset = ch;
}


shared_ptr <const contentHandler> htmlTextPart::getPlainText() const
{
	return m_plainText;
}


void htmlTextPart::setPlainText(shared_ptr <contentHandler> plainText)
{
	m_plainText = plainText->clone();
}


const shared_ptr <const contentHandler> htmlTextPart::getText() const
{
	return m_text;
}


void htmlTextPart::setText(shared_ptr <contentHandler> text)
{
	m_text = text->clone();
}


size_t htmlTextPart::getObjectCount() const
{
	return m_objects.size();
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::getObjectAt(const size_t pos) const
{
	return m_objects[pos];
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::findObject(const string& id) const
{
	for (std::vector <shared_ptr <embeddedObject> >::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->matchesId(id))
			return *o;
	}

	return null;
}


bool htmlTextPart::hasObject(const string& id) const
{
	for (std::vector <shared_ptr <embeddedObject> >::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o)
	{
		if ((*o)->matchesId(id))
			return true;
	}

	return false;
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(shared_ptr <contentHandler> data, const vmime::encoding& enc, const mediaType& type)
{
	const messageId mid(messageId::generateId());

	shared_ptr <embeddedObject> obj = make_shared <embeddedObject>
		(data, enc, mid.getId(), type, embeddedObject::REFERENCED_BY_ID);

	m_objects.push_back(obj);

	return obj;
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(shared_ptr <contentHandler> data, const mediaType& type)
{
	return addObject(data, encoding::decide(data), type);
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject
	(const string& data, const mediaType& type)
{
	shared_ptr <stringContentHandler> cts = make_shared <stringContentHandler>(data);
	return addObject(cts, encoding::decide(cts), type);
}



//
// htmlTextPart::embeddedObject
//

htmlTextPart::embeddedObject::embeddedObject
	(shared_ptr <contentHandler> data, const encoding& enc,
	 const string& id, const mediaType& type, const ReferenceType refType)
	: m_data(vmime::clone(data)),
	  m_encoding(enc), m_id(id), m_type(type), m_refType(refType)
{
}


shared_ptr <const contentHandler> htmlTextPart::embeddedObject::getData() const
{
	return m_data;
}


const vmime::encoding htmlTextPart::embeddedObject::getEncoding() const
{
	return m_encoding;
}


const string htmlTextPart::embeddedObject::getId() const
{
	return m_id;
}


const string htmlTextPart::embeddedObject::getReferenceId() const
{
	if (m_refType == REFERENCED_BY_ID)
		return string("cid:") + m_id;
	else
		return m_id;
}


const mediaType htmlTextPart::embeddedObject::getType() const
{
	return m_type;
}


htmlTextPart::embeddedObject::ReferenceType htmlTextPart::embeddedObject::getReferenceType() const
{
	return m_refType;
}


bool htmlTextPart::embeddedObject::matchesId(const string& id) const
{
	if (m_refType == REFERENCED_BY_ID)
		return m_id == cleanId(id);
	else
		return m_id == id;
}


// static
const string htmlTextPart::embeddedObject::cleanId(const string& id)
{
	if (id.length() >= 4 &&
	    (id[0] == 'c' || id[0] == 'C') &&
	    (id[1] == 'i' || id[1] == 'I') &&
	    (id[2] == 'd' || id[2] == 'D') &&
	    id[3] == ':')
	{
		return id.substr(4);
	}
	else
	{
		return id;
	}
}


} // vmime
