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

#include "vmime/net/imap/IMAPStructure.hpp"
#include "vmime/net/imap/IMAPPart.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPStructure::IMAPStructure()
{
}


IMAPStructure::IMAPStructure(const IMAPParser::body* body)
{
	m_parts.push_back(IMAPPart::create(NULL, 0, body));
}


IMAPStructure::IMAPStructure(ref <IMAPPart> parent, const std::vector <IMAPParser::body*>& list)
{
	int number = 0;

	for (std::vector <IMAPParser::body*>::const_iterator
	     it = list.begin() ; it != list.end() ; ++it, ++number)
	{
		m_parts.push_back(IMAPPart::create(parent, number, *it));
	}
}


ref <const part> IMAPStructure::getPartAt(const int x) const
{
	return m_parts[x];
}


ref <part> IMAPStructure::getPartAt(const int x)
{
	return m_parts[x];
}


int IMAPStructure::getPartCount() const
{
	return m_parts.size();
}


// static
ref <IMAPStructure> IMAPStructure::emptyStructure()
{
	static ref <IMAPStructure> emptyStructure = vmime::create <IMAPStructure>();
	return emptyStructure;
}


} // imap
} // net
} // vmime

