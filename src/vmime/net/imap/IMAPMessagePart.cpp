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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPMessagePart.hpp"
#include "vmime/net/imap/IMAPMessageStructure.hpp"

#include "vmime/attachmentHelper.hpp"
#include "vmime/contentDispositionField.hpp"
#include "vmime/contentTypeField.hpp"


namespace vmime {
namespace net {
namespace imap {


namespace {

	// Set the parameters of a field from a list of parameters of the
	// body structure
	void parseParameters(
		parameterizedHeaderField& field,
		const IMAPParser::body_fld_param* params
	) {

		if (!params) {
			return;
		}

		std::vector <std::pair <string, string> > rawParams;

		for (const auto& param : params->items) {
			rawParams.push_back(std::make_pair(param->string1->value, param->string2->value));
		}

		// Servers should decode RFC-2231 parameter values and return them
		// in UTF-8 (RFC 9051, section 7.5.2), which is how 8-bit values
		// are parsed anyway
		field.parseParameters(parsingContext::getDefaultContext(), rawParams);
	}

	// Return the body location (RFC 3501), which is the first extension
	// data after the body language, or NULL if there is none
	const IMAPParser::nstring* getBodyLocation(
		const std::vector <std::unique_ptr <IMAPParser::body_extension>>& exts
	) {

		if (!exts.empty() && exts[0]->nstring && !exts[0]->nstring->isNIL) {
			return exts[0]->nstring.get();
		}

		return NULL;
	}

	// Create a header with the MIME fields which are known from the
	// body structure, so that the rules of attachmentHelper can be used
	shared_ptr <header> createStructureHeader(
		const mediaType& type,
		const IMAPParser::body_fld_param* typeParams,
		const IMAPParser::body_fld_dsp* disp,
		const IMAPParser::nstring* id,
		const IMAPParser::nstring* location
	) {

		shared_ptr <header> hdr = make_shared <header>();

		shared_ptr <contentTypeField> ctf =
			hdr->getField <contentTypeField>(fields::CONTENT_TYPE);

		ctf->setValue(type);
		parseParameters(*ctf, typeParams);

		if (disp && disp->str()) {

			shared_ptr <contentDispositionField> cdf =
				hdr->getField <contentDispositionField>(fields::CONTENT_DISPOSITION);

			cdf->setValue(contentDisposition(disp->str()->value));
			parseParameters(*cdf, disp->body_fld_param());
		}

		if (id && !id->value.empty()) {
			hdr->ContentId()->parse(id->value);
		}

		if (location && !location->value.empty()) {
			hdr->ContentLocation()->parse(location->value);
		}

		return hdr;
	}
}


IMAPMessagePart::IMAPMessagePart(
	const shared_ptr <IMAPMessagePart>& parent,
	const size_t number,
	const IMAPParser::body_type_mpart* mpart
)
	: m_parent(parent),
	  m_header(null),
	  m_number(number),
	  m_size(0) {

	m_mediaType = vmime::mediaType(
		"multipart",
		mpart->media_subtype->value
	);

	// Extension data, which is not returned for a non-extensible
	// "BODY" fetch
	const IMAPParser::body_ext_mpart* ext = mpart->body_ext_mpart.get();

	if (ext && ext->body_fld_dsp && ext->body_fld_dsp->str()) {
		m_dispType = contentDisposition(ext->body_fld_dsp->str()->value);
	}

	m_structureHeader = createStructureHeader(
		m_mediaType,
		ext ? ext->body_fld_param.get() : NULL,
		ext ? ext->body_fld_dsp.get() : NULL,
		NULL,
		ext ? getBodyLocation(ext->body_extensions) : NULL
	);
}


IMAPMessagePart::IMAPMessagePart(
	const shared_ptr <IMAPMessagePart>& parent,
	const size_t number,
	const IMAPParser::body_type_1part* part
)
	: m_parent(parent),
	  m_header(null),
	  m_number(number),
	  m_size(0) {

	const IMAPParser::body_fields* bodyFields;

	if (part->body_type_text) {

		m_mediaType = vmime::mediaType(
			"text",
			part->body_type_text->media_text->media_subtype->value
		);

		bodyFields = part->body_type_text->body_fields.get();

	} else if (part->body_type_msg) {

		m_mediaType = vmime::mediaType(
			"message",
			part->body_type_msg->media_message->media_subtype->value
		);

		bodyFields = part->body_type_msg->body_fields.get();

	} else {

		m_mediaType = vmime::mediaType(
			part->body_type_basic->media_basic->media_type->value,
			part->body_type_basic->media_basic->media_subtype->value
		);

		bodyFields = part->body_type_basic->body_fields.get();
	}

	m_size = bodyFields->body_fld_octets->value;

	// Extension data, which is not returned for a non-extensible
	// "BODY" fetch
	const IMAPParser::body_ext_1part* ext = part->body_ext_1part.get();

	if (ext && ext->body_fld_dsp && ext->body_fld_dsp->str()) {
		m_dispType = contentDisposition(ext->body_fld_dsp->str()->value);
	}

	m_structureHeader = createStructureHeader(
		m_mediaType,
		bodyFields->body_fld_param.get(),
		ext ? ext->body_fld_dsp.get() : NULL,
		bodyFields->body_fld_id.get(),
		ext ? getBodyLocation(ext->body_extensions) : NULL
	);

	m_structure = null;
}


shared_ptr <const messageStructure> IMAPMessagePart::getStructure() const {

	if (m_structure) {
		return m_structure;
	} else {
		return IMAPMessageStructure::emptyStructure();
	}
}


shared_ptr <messageStructure> IMAPMessagePart::getStructure() {

	if (m_structure) {
		return m_structure;
	} else {
		return IMAPMessageStructure::emptyStructure();
	}
}


shared_ptr <const IMAPMessagePart> IMAPMessagePart::getParent() const {

	return m_parent.lock();
}


const mediaType& IMAPMessagePart::getType() const {

	return m_mediaType;
}


const contentDisposition &IMAPMessagePart::getDisposition() const {

	return m_dispType;
}


size_t IMAPMessagePart::getSize() const {

	return m_size;
}


size_t IMAPMessagePart::getNumber() const {

	return m_number;
}


word IMAPMessagePart::getName() const {

	return attachmentHelper::getAttachmentName(*m_structureHeader);
}


bool IMAPMessagePart::isAttachment(const unsigned int options) const {

	return attachmentHelper::isAttachment(*m_structureHeader, !m_parent.lock(), options);
}


shared_ptr <const header> IMAPMessagePart::getHeader() const {

	if (!m_header) {
		throw exceptions::unfetched_object();
	} else {
		return m_header;
	}
}


// static
shared_ptr <IMAPMessagePart> IMAPMessagePart::create(
	const shared_ptr <IMAPMessagePart>& parent,
	const size_t number,
	const IMAPParser::body* body
) {

	if (body->body_type_mpart) {

		auto part = make_shared <IMAPMessagePart>(parent, number, body->body_type_mpart.get());
		part->m_structure = make_shared <IMAPMessageStructure>(part, body->body_type_mpart->list);

		return part;

	} else {

		return make_shared <IMAPMessagePart>(parent, number, body->body_type_1part.get());
	}
}


header& IMAPMessagePart::getOrCreateHeader() {

	if (m_header) {
		return *m_header;
	} else {
		return *(m_header = make_shared <header>());
	}
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

