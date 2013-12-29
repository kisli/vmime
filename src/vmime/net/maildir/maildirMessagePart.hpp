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

#ifndef VMIME_NET_MAILDIR_MAILDIRMESSAGEPART_HPP_INCLUDED
#define VMIME_NET_MAILDIR_MAILDIRMESSAGEPART_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/message.hpp"


namespace vmime {
namespace net {
namespace maildir {


class maildirMessageStructure;


class maildirMessagePart : public messagePart
{
public:

	maildirMessagePart(shared_ptr <maildirMessagePart> parent, const int number, const bodyPart& part);
	~maildirMessagePart();


	shared_ptr <const messageStructure> getStructure() const;
	shared_ptr <messageStructure> getStructure();

	weak_ptr <const maildirMessagePart> getParent() const { return (m_parent); }

	const mediaType& getType() const;
	size_t getSize() const;
	int getNumber() const;

	shared_ptr <const header> getHeader() const;

	header& getOrCreateHeader();

	size_t getHeaderParsedOffset() const;
	size_t getHeaderParsedLength() const;

	size_t getBodyParsedOffset() const;
	size_t getBodyParsedLength() const;

	void initStructure(const bodyPart& part);

private:

	shared_ptr <maildirMessageStructure> m_structure;
	weak_ptr <maildirMessagePart> m_parent;
	shared_ptr <header> m_header;

	int m_number;
	size_t m_size;
	mediaType m_mediaType;

	size_t m_headerParsedOffset;
	size_t m_headerParsedLength;

	size_t m_bodyParsedOffset;
	size_t m_bodyParsedLength;
};


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_MAILDIRMESSAGEPART_HPP_INCLUDED
