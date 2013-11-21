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

#ifndef VMIME_NET_IMAP_IMAPMESSAGESTRUCTURE_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPMESSAGESTRUCTURE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/message.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPMessagePart;


class VMIME_EXPORT IMAPMessageStructure : public messageStructure
{
public:

	IMAPMessageStructure();
	IMAPMessageStructure(const IMAPParser::body* body);
	IMAPMessageStructure(shared_ptr <IMAPMessagePart> parent, const std::vector <IMAPParser::body*>& list);

	shared_ptr <const messagePart> getPartAt(const size_t x) const;
	shared_ptr <messagePart> getPartAt(const size_t x);
	size_t getPartCount() const;

	static shared_ptr <IMAPMessageStructure> emptyStructure();

private:

	std::vector <shared_ptr <IMAPMessagePart> > m_parts;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPMESSAGESTRUCTURE_HPP_INCLUDED

