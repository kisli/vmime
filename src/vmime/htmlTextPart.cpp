//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include "vmime/parserHelpers.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/uriReference.hpp"
#include "vmime/utility/details/classicOStringStream.hpp"


namespace vmime {


// Base URI when no other one applies (RFC 2557, section 5)
static const char* const DEFAULT_BASE_URI = "thismessage:/";


static string resolveURI(const string& base, const string& ref) {

	return utility::uriReference(ref).resolve(utility::uriReference(base)).toString();
}


// "cid:" URIs only reference Content-Ids (RFC 2557, section 8.3)
static bool isCIDReference(const string& ref) {

	return utility::stringUtils::isStringEqualNoCase(utility::uriReference(ref).getScheme(), "cid");
}


static bool isMultipartRelated(const bodyPart& part) {

	const shared_ptr <const mediaType> type =
		part.getHeader()->findFieldValue <mediaType>(fields::CONTENT_TYPE);

	return type &&
	       type->getType() == mediaTypes::MULTIPART &&
	       type->getSubType() == mediaTypes::MULTIPART_RELATED;
}


// Returns the URI in the Content-Location field of a part: encoded words are
// decoded, and white-spaces are removed to unfold it (RFC 2557, section 4.4.3,
// and RFC 2017, section 3.1)
static string getLocation(const bodyPart& part) {

	const shared_ptr <const text> location =
		part.getHeader()->findFieldValue <text>(fields::CONTENT_LOCATION);

	if (!location) {
		return "";
	}

	const string buffer = location->getWholeBuffer();

	string uri;
	uri.reserve(buffer.length());

	for (const char c : buffer) {

		if (!parserHelpers::isSpace(c)) {
			uri += c;
		}
	}

	return uri;
}


// Returns the base URI of the relative URIs in a part: the first absolute URI
// in the Content-Location fields of this part and of the parts which contain
// it, else "thismessage:/" (RFC 2557, section 5)
static string findBaseURI(const bodyPart* part) {

	for ( ; part ; part = part->getParentPart()) {

		const string location = getLocation(*part);

		if (utility::uriReference(location).hasScheme()) {
			return location;
		}
	}

	return DEFAULT_BASE_URI;
}


// Returns the "href" attribute of the BASE element of an HTML document, or an
// empty string if there is none (RFC 2557, section 5); character references
// are not decoded
static string findHTMLBase(const string& html) {

	const string lower = utility::stringUtils::toLower(html);
	const size_t length = lower.length();

	for (size_t pos = lower.find("<base") ; pos != string::npos ; pos = lower.find("<base", pos + 1)) {

		size_t p = pos + 5;

		// Not "<basefont", for example
		if (p < length && !parserHelpers::isSpace(lower[p]) && lower[p] != '/' && lower[p] != '>') {
			continue;
		}

		// Attributes, until the end of the tag
		while (p < length && lower[p] != '>') {

			if (parserHelpers::isSpace(lower[p]) || lower[p] == '/') {
				++p;
				continue;
			}

			const size_t nameStart = p;

			while (p < length && !parserHelpers::isSpace(lower[p]) &&
			       lower[p] != '=' && lower[p] != '>' && lower[p] != '/') {

				++p;
			}

			const string name = lower.substr(nameStart, p - nameStart);

			while (p < length && parserHelpers::isSpace(lower[p])) {
				++p;
			}

			string value;

			if (p < length && lower[p] == '=') {

				++p;

				while (p < length && parserHelpers::isSpace(lower[p])) {
					++p;
				}

				// Quoted value
				if (p < length && (lower[p] == '"' || lower[p] == '\'')) {

					const size_t end = std::min(lower.find(lower[p], p + 1), length);

					value = html.substr(p + 1, end - p - 1);
					p = std::min(end + 1, length);

				// Unquoted value
				} else {

					const size_t valueStart = p;

					while (p < length && !parserHelpers::isSpace(lower[p]) && lower[p] != '>') {
						++p;
					}

					value = html.substr(valueStart, p - valueStart);
				}
			}

			if (name == "href") {
				return utility::stringUtils::trim(value);
			}
		}
	}

	return "";
}


// Finds the parts with a Content-Location in a multipart/related part, except
// the HTML text, without looking into nested multipart/related parts: RFC 2557
// does not cover references from outside to their parts (section 7)
static void findLocationParts(
	const bodyPart& part,
	const bodyPart& textPart,
	std::vector <shared_ptr <const bodyPart> >& locParts
) {

	for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i) {

		const shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

		if (p->getBody()->getPartCount() != 0) {

			if (!isMultipartRelated(*p)) {
				findLocationParts(*p, textPart, locParts);
			}

		} else if (p.get() != &textPart) {

			const string location = getLocation(*p);

			if (!location.empty() && !isCIDReference(location)) {
				locParts.push_back(p);
			}
		}
	}
}


// Long URIs are split into words of at most 40 bytes, where the field can be
// folded (RFC 2557, section 4.4.2, and RFC 2017, section 3.1); URIs with 8-bit
// bytes are encoded with the charset of the HTML text (section 4.4.1)
static text locationToText(const string& location, const charset& ch) {

	if (!utility::stringUtils::is7bit(location)) {
		return text(word(location, ch));
	}

	// "Content-Location: " is 18 bytes long
	if (18 + location.length() <= lineLengthLimits::convenient) {
		return text(word(location, charset(charsets::US_ASCII)));
	}

	string words;

	for (size_t pos = 0 ; pos < location.length() ; pos += 40) {

		if (pos != 0) {
			words += ' ';
		}

		words += location.substr(pos, 40);
	}

	return text(word(words, charset(charsets::US_ASCII)));
}


htmlTextPart::htmlTextPart()
	: m_plainText(make_shared <emptyContentHandler>()),
	  m_text(make_shared <emptyContentHandler>()),
	  m_baseURI(DEFAULT_BASE_URI) {

}


htmlTextPart::~htmlTextPart() {

}


mediaType htmlTextPart::getType() const {

	return mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML);
}


size_t htmlTextPart::getPartCount() const {

	return m_plainText->isEmpty() ? 1 : 2;
}


void htmlTextPart::generateIn(
	const shared_ptr <bodyPart>& /* message */,
	const shared_ptr <bodyPart>& parent
) const {

	// Plain text
	if (!m_plainText->isEmpty()) {

		// -- Create a new part
		shared_ptr <bodyPart> part = make_shared <bodyPart>();
		parent->getBody()->appendPart(part);

		// -- Set contents
		part->getBody()->setContents(
			m_plainText,
			mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN),
			m_charset,
			encoding::decide(m_plainText, m_charset, encoding::USAGE_TEXT)
		);
	}

	// HTML text
	// -- Create a new part
	shared_ptr <bodyPart> htmlPart = make_shared <bodyPart>();

	// -- Set contents
	htmlPart->getBody()->setContents(
		m_text,
		mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML),
		m_charset,
		encoding::decide(m_text, m_charset, encoding::USAGE_TEXT)
	);

	// Handle the case we have embedded objects
	if (!m_objects.empty()) {

		// Create a "multipart/related" body part
		shared_ptr <bodyPart> relPart = make_shared <bodyPart>();
		parent->getBody()->appendPart(relPart);

		relPart->getHeader()->ContentType()->setValue(
			mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_RELATED)
		);

		// The type of the root part is required (RFC 2387, section 3.1)
		dynamicCast <contentTypeField>(relPart->getHeader()->ContentType())
			->getParameter("type")->setValue(word(getType().generate(), charset(charsets::US_ASCII)));

		// Add the HTML part into this part
		relPart->getBody()->appendPart(htmlPart);

		// Also add objects into this part
		for (std::vector <shared_ptr <embeddedObject> >::const_iterator it = m_objects.begin() ;
		     it != m_objects.end() ; ++it) {

			shared_ptr <bodyPart> objPart = make_shared <bodyPart>();
			relPart->getBody()->appendPart(objPart);

			objPart->getHeader()->ContentType()->setValue((*it)->getType());

			if ((*it)->getReferenceType() == embeddedObject::REFERENCED_BY_ID) {

				string id = (*it)->getId();

				if (id.length() >= 4 &&
				    (id[0] == 'c' || id[0] == 'C') &&
				    (id[1] == 'i' || id[1] == 'I') &&
				    (id[2] == 'd' || id[2] == 'D') &&
				    id[3] == ':') {

					id = id.substr(4);
				}

				objPart->getHeader()->ContentId()->setValue(messageId("<" + id + ">"));
			}

			// An object can be referenced by both a Content-Id and a URI
			if (!(*it)->m_location.empty()) {
				objPart->getHeader()->ContentLocation()->setValue(locationToText((*it)->m_location, m_charset));
			}

			objPart->getHeader()->ContentDisposition()->setValue(contentDisposition(contentDispositionTypes::INLINE));
			objPart->getHeader()->ContentTransferEncoding()->setValue((*it)->getEncoding());
				//encoding(encodingTypes::BASE64);

			objPart->getBody()->setContents((*it)->getData()->clone());
		}

	} else {

		// Add the HTML part into the parent part
		parent->getBody()->appendPart(htmlPart);
	}
}


void htmlTextPart::findEmbeddedParts(
	const bodyPart& part,
	std::vector <shared_ptr <const bodyPart> >& cidParts
) {

	for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i) {

		shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

		if (p->getHeader()->hasField(fields::CONTENT_ID)) {
			cidParts.push_back(p);
		}

		findEmbeddedParts(*p, cidParts);
	}
}


shared_ptr <htmlTextPart::embeddedObject> htmlTextPart::addEmbeddedObject(
	const bodyPart& part,
	const string& id,
	const embeddedObject::ReferenceType refType
) {

	mediaType type;

	shared_ptr <const headerField> ctf =
		part.getHeader()->findField(fields::CONTENT_TYPE);

	if (ctf) {

		type = *ctf->getValue <mediaType>();

	} else {

		// No "Content-type" field: assume "application/octet-stream".
	}

	shared_ptr <embeddedObject> obj = make_shared <embeddedObject>(
		vmime::clone(part.getBody()->getContents()),
		part.getBody()->getEncoding(),
		id,
		type,
		refType
	);

	m_objects.push_back(obj);

	return obj;
}


void htmlTextPart::parse(
	const shared_ptr <const bodyPart>& message,
	const shared_ptr <const bodyPart>& parent,
	const shared_ptr <const bodyPart>& textPart
) {

	// Search for possible embedded objects in the _whole_ message.
	std::vector <shared_ptr <const bodyPart> > cidParts;

	findEmbeddedParts(*message, cidParts);

	// Objects referenced by a URI are in the multipart/related part of the
	// HTML text, or in a multipart/related part which contains it (RFC 2557,
	// sections 7 and 8.2)
	std::vector <shared_ptr <const bodyPart> > locParts;

	for (const bodyPart* p = textPart->getParentPart() ; p ; p = p->getParentPart()) {

		if (isMultipartRelated(*p)) {
			findLocationParts(*p, *textPart, locParts);
		}
	}

	// Extract HTML text
	utility::details::classicOStringStream oss;
	utility::outputStreamAdapter adapter(oss);

	textPart->getBody()->getContents()->extract(adapter);

	const string data = oss.str();

	m_text = textPart->getBody()->getContents()->clone();

	// Find charset
	shared_ptr <const contentTypeField> ctf =
		textPart->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf && ctf->hasCharset()) {
		m_charset = ctf->getCharset();
	} else {
		m_charset = charset();
	}

	// Base URI of the relative URIs in the HTML text: the BASE element comes
	// first, and it can itself be relative (RFC 2557, section 5)
	m_baseURI = findBaseURI(textPart.get());

	const string htmlBase = findHTMLBase(data);

	if (!htmlBase.empty()) {
		m_baseURI = resolveURI(m_baseURI, htmlBase);
	}

	// Sets the Content-Location of an object, and the URI it resolves to
	auto setLocation = [](embeddedObject& obj, const bodyPart& part) {

		obj.m_location = getLocation(part);
		obj.m_resolvedLocation = resolveURI(findBaseURI(part.getParentPart()), obj.m_location);
	};

	// Extract embedded objects. For each part with a Content-Id, we check if
	// its CID is contained in the HTML text.
	for (std::vector <shared_ptr <const bodyPart> >::const_iterator p = cidParts.begin() ;
	     p != cidParts.end() ; ++p) {

		const shared_ptr <const headerField> midField =
			(*p)->getHeader()->findField(fields::CONTENT_ID);

		const messageId mid = *midField->getValue <messageId>();

		if (data.find("CID:" + mid.getId()) != string::npos ||
		    data.find("cid:" + mid.getId()) != string::npos) {

			// This part is referenced in the HTML text.
			// Add it to the embedded object list.
			shared_ptr <embeddedObject> obj =
				addEmbeddedObject(**p, mid.getId(), embeddedObject::REFERENCED_BY_ID);

			// The object may also be identified by a Content-Location. In
			// this case, there is a single embedded object, referenced by
			// its Content-Id, which can also be found with its URI.
			const std::vector <shared_ptr <const bodyPart> >::iterator loc =
				std::find(locParts.begin(), locParts.end(), *p);

			if (loc != locParts.end()) {

				setLocation(*obj, **p);
				locParts.erase(loc);
			}
		}
	}

	// The other parts with a URI are objects referenced by their URI: which
	// URIs are used is up to the HTML text, so they are all kept
	for (std::vector <shared_ptr <const bodyPart> >::const_iterator p = locParts.begin() ;
	     p != locParts.end() ; ++p) {

		shared_ptr <embeddedObject> obj =
			addEmbeddedObject(**p, getLocation(**p), embeddedObject::REFERENCED_BY_LOCATION);

		setLocation(*obj, **p);
	}

	// Extract plain text, if any.
	if (!findPlainTextPart(*message, *parent, *textPart)) {

		m_plainText = make_shared <emptyContentHandler>();
	}
}


bool htmlTextPart::findPlainTextPart(
	const bodyPart& part,
	const bodyPart& parent,
	const bodyPart& textPart
) {

	// We search for the nearest "multipart/alternative" part.
	const shared_ptr <const headerField> ctf =
		part.getHeader()->findField(fields::CONTENT_TYPE);

	if (ctf) {

		const mediaType type = *ctf->getValue <mediaType>();

		if (type.getType() == mediaTypes::MULTIPART &&
		    type.getSubType() == mediaTypes::MULTIPART_ALTERNATIVE) {

			shared_ptr <const bodyPart> foundPart;

			for (size_t i = 0 ; i < part.getBody()->getPartCount() ; ++i) {

				const shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

				if (p.get() == &parent ||     // if "text/html" is in "multipart/related"
				    p.get() == &textPart) {   // if not...

					foundPart = p;
				}
			}

			if (foundPart) {

				bool found = false;

				// Now, search for the alternative plain text part
				for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i) {

					const shared_ptr <const bodyPart> p = part.getBody()->getPartAt(i);

					const shared_ptr <const headerField> ctf =
						p->getHeader()->findField(fields::CONTENT_TYPE);

					if (ctf) {

						const mediaType type = *ctf->getValue <mediaType>();

						if (type.getType() == mediaTypes::TEXT &&
						    type.getSubType() == mediaTypes::TEXT_PLAIN) {

							m_plainText = p->getBody()->getContents()->clone();
							found = true;
						}

					} else {

						// No "Content-type" field.
					}
				}

				// If we don't have found the plain text part here, it means that
				// it does not exists (the MUA which built this message probably
				// did not include it...).
				return found;
			}
		}

	} else {

		// No "Content-type" field.
	}

	bool found = false;

	for (size_t i = 0 ; !found && i < part.getBody()->getPartCount() ; ++i) {

		found = findPlainTextPart(*part.getBody()->getPartAt(i), parent, textPart);
	}

	return found;
}


const charset& htmlTextPart::getCharset() const {

	return m_charset;
}


void htmlTextPart::setCharset(const charset& ch) {

	m_charset = ch;
}


shared_ptr <const contentHandler> htmlTextPart::getPlainText() const {

	return m_plainText;
}


void htmlTextPart::setPlainText(const shared_ptr <contentHandler>& plainText) {

	m_plainText = plainText->clone();
}


shared_ptr <const contentHandler> htmlTextPart::getText() const {

	return m_text;
}


void htmlTextPart::setText(const shared_ptr <contentHandler>& text) {

	m_text = text->clone();
}


size_t htmlTextPart::getObjectCount() const {

	return m_objects.size();
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::getObjectAt(const size_t pos) const {

	return m_objects[pos];
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::findObject(const string& id) const
{
	// URIs other than "cid:" ones are compared with the Content-Location of
	// objects, once resolved (RFC 2557, section 8.2)
	const bool isCID = isCIDReference(id);
	const string uri = resolveURI(m_baseURI, id);

	for (std::vector <shared_ptr <embeddedObject> >::const_iterator o = m_objects.begin() ;
	     o != m_objects.end() ; ++o) {

		if ((*o)->getReferenceType() == embeddedObject::REFERENCED_BY_ID && (*o)->matchesId(id)) {
			return *o;
		}

		if (!isCID && !(*o)->m_resolvedLocation.empty() && (*o)->m_resolvedLocation == uri) {
			return *o;
		}
	}

	return null;
}


bool htmlTextPart::hasObject(const string& id) const {

	return !!findObject(id);
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject(
	const shared_ptr <contentHandler>& data,
	const vmime::encoding& enc,
	const mediaType& type
) {

	const messageId mid(messageId::generateId());

	shared_ptr <embeddedObject> obj = make_shared <embeddedObject>(
		data, enc, mid.getId(), type, embeddedObject::REFERENCED_BY_ID
	);

	m_objects.push_back(obj);

	return obj;
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject(
	const shared_ptr <contentHandler>& data,
	const mediaType& type
) {

	return addObject(data, encoding::decide(data), type);
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject(
	const string& data,
	const mediaType& type
) {

	shared_ptr <stringContentHandler> cts = make_shared <stringContentHandler>(data);
	return addObject(cts, encoding::decide(cts), type);
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject(
	const shared_ptr <contentHandler>& data,
	const vmime::encoding& enc,
	const mediaType& type,
	const string& location
) {

	// A URI has no white-spaces: they are removed from a folded
	// Content-Location field when it is parsed (RFC 2557, section 4.4.3)
	if (location.empty()) {
		throw exceptions::invalid_argument();
	}

	for (const char c : location) {

		if (c == ' ' || parserHelpers::isControl(c)) {
			throw exceptions::invalid_argument();
		}
	}

	shared_ptr <embeddedObject> obj = make_shared <embeddedObject>(
		data, enc, location, type, embeddedObject::REFERENCED_BY_LOCATION
	);

	obj->m_location = location;
	obj->m_resolvedLocation = resolveURI(m_baseURI, location);

	m_objects.push_back(obj);

	return obj;
}


shared_ptr <const htmlTextPart::embeddedObject> htmlTextPart::addObject(
	const shared_ptr <contentHandler>& data,
	const mediaType& type,
	const string& location
) {

	return addObject(data, encoding::decide(data), type, location);
}



//
// htmlTextPart::embeddedObject
//

htmlTextPart::embeddedObject::embeddedObject(
	const shared_ptr <contentHandler>& data,
	const encoding& enc,
	const string& id,
	const mediaType& type,
	const ReferenceType refType
)
	: m_data(vmime::clone(data)),
	  m_encoding(enc),
	  m_id(id),
	  m_type(type),
	  m_refType(refType) {

}


shared_ptr <const contentHandler> htmlTextPart::embeddedObject::getData() const {

	return m_data;
}


vmime::encoding htmlTextPart::embeddedObject::getEncoding() const {

	return m_encoding;
}


string htmlTextPart::embeddedObject::getId() const {

	return m_id;
}


string htmlTextPart::embeddedObject::getReferenceId() const {

	if (m_refType == REFERENCED_BY_ID) {
		return string("cid:") + m_id;
	} else {
		return m_id;
	}
}


mediaType htmlTextPart::embeddedObject::getType() const {

	return m_type;
}


htmlTextPart::embeddedObject::ReferenceType htmlTextPart::embeddedObject::getReferenceType() const {

	return m_refType;
}


bool htmlTextPart::embeddedObject::matchesId(const string& id) const {

	if (m_refType == REFERENCED_BY_ID) {
		return m_id == cleanId(id);
	} else {
		return m_id == id;
	}
}


// static
string htmlTextPart::embeddedObject::cleanId(const string& id) {

	if (id.length() >= 4 &&
	    (id[0] == 'c' || id[0] == 'C') &&
	    (id[1] == 'i' || id[1] == 'I') &&
	    (id[2] == 'd' || id[2] == 'D') &&
	    id[3] == ':') {

		return id.substr(4);

	} else {

		return id;
	}
}


} // vmime
