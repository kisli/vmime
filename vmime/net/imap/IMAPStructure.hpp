//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_NET_IMAP_IMAPSTRUCTURE_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPSTRUCTURE_HPP_INCLUDED


#include "vmime/net/message.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPPart;


class IMAPStructure : public structure
{
public:

	IMAPStructure();
	IMAPStructure(const IMAPParser::body* body);
	IMAPStructure(ref <IMAPPart> parent, const std::vector <IMAPParser::body*>& list);

	ref <const part> getPartAt(const int x) const;
	ref <part> getPartAt(const int x);
	int getPartCount() const;

	static ref <IMAPStructure> emptyStructure();

private:

	std::vector <ref <IMAPPart> > m_parts;
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPSTRUCTURE_HPP_INCLUDED

