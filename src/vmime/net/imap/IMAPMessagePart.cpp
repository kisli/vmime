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


namespace vmime {
namespace net {
namespace imap {


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

	if (part->body_type_text) {

		m_mediaType = vmime::mediaType(
			"text",
			part->body_type_text->media_text->media_subtype->value
		);

		m_size = part->body_type_text->body_fields->body_fld_octets->value;

	} else if (part->body_type_msg) {

		m_mediaType = vmime::mediaType(
			"message",
			part->body_type_msg->media_message->media_subtype->value
		);

	} else {

		m_mediaType = vmime::mediaType(
			part->body_type_basic->media_basic->media_type->value,
			part->body_type_basic->media_basic->media_subtype->value
		);

		m_size = part->body_type_basic->body_fields->body_fld_octets->value;

		if (const auto* pparam = part->body_type_basic->body_fields->body_fld_param.get()) {
			for (const auto& param : pparam->items) {
				if (param->string1->value == "NAME") {
					m_name = param->string2->value;
				}
			}
		}
	}

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


size_t IMAPMessagePart::getSize() const {

	return m_size;
}


size_t IMAPMessagePart::getNumber() const {

	return m_number;
}


string IMAPMessagePart::getName() const {

	return m_name;
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

