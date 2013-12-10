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

#ifndef VMIME_NET_IMAP_IMAPMESSAGEPART_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPMESSAGEPART_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/message.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPMessageStructure;


class VMIME_EXPORT IMAPMessagePart : public messagePart
{
public:

	IMAPMessagePart(shared_ptr <IMAPMessagePart> parent, const int number, const IMAPParser::body_type_mpart* mpart);
	IMAPMessagePart(shared_ptr <IMAPMessagePart> parent, const int number, const IMAPParser::body_type_1part* part);

	shared_ptr <const messageStructure> getStructure() const;
	shared_ptr <messageStructure> getStructure();

	shared_ptr <const IMAPMessagePart> getParent() const;

	const mediaType& getType() const;
	size_t getSize() const;
	int getNumber() const;

	shared_ptr <const header> getHeader() const;


	static shared_ptr <IMAPMessagePart> create
		(shared_ptr <IMAPMessagePart> parent, const int number, const IMAPParser::body* body);


	header& getOrCreateHeader();

private:

	shared_ptr <IMAPMessageStructure> m_structure;
	weak_ptr <IMAPMessagePart> m_parent;
	shared_ptr <header> m_header;

	int m_number;
	size_t m_size;
	mediaType m_mediaType;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPMESSAGEPART_HPP_INCLUDED

