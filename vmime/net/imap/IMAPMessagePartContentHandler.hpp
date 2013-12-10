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

#ifndef VMIME_NET_IMAP_IMAPMESSAGEPARTCONTENTHANDLER_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPMESSAGEPARTCONTENTHANDLER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/contentHandler.hpp"
#include "vmime/net/imap/IMAPMessage.hpp"


namespace vmime {
namespace net {
namespace imap {


class VMIME_EXPORT IMAPMessagePartContentHandler : public contentHandler
{
public:

	IMAPMessagePartContentHandler(shared_ptr <IMAPMessage> msg, shared_ptr <messagePart> part, const vmime::encoding& encoding);

	shared_ptr <contentHandler> clone() const;

	void generate(utility::outputStream& os, const vmime::encoding& enc, const size_t maxLineLength = lineLengthLimits::infinite) const;

	void extract(utility::outputStream& os, utility::progressListener* progress = NULL) const;
	void extractRaw(utility::outputStream& os, utility::progressListener* progress = NULL) const;

	size_t getLength() const;

	bool isEncoded() const;

	const vmime::encoding& getEncoding() const;

	bool isEmpty() const;

	bool isBuffered() const;

	void setContentTypeHint(const mediaType& type);
	const mediaType getContentTypeHint() const;

private:

	weak_ptr <IMAPMessage> m_message;
	weak_ptr <messagePart> m_part;

	vmime::encoding m_encoding;
	vmime::mediaType m_contentType;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPMESSAGEPARTCONTENTHANDLER_HPP_INCLUDED

