//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "mailboxList.hpp"


namespace vmime
{


// Address insertion
void mailboxList::append(const address& addr)
{
	// Ensure this is a "mailbox" object
	const mailbox& mb = dynamic_cast<const mailbox&>(addr);

	m_list.push_back(mb.clone());
}


void mailboxList::insert(const iterator it, const address& addr)
{
	// Ensure this is a "mailbox" object
	const mailbox& mb = dynamic_cast<const mailbox&>(addr);

	m_list.insert(it.m_iterator, mb.clone());
}


} // vmime
