//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include <ostream>


namespace std
{


std::ostream& operator<<(std::ostream& os, const vmime::charset& ch)
{
	os << "[charset: " << ch.getName() << "]";
	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::text& txt)
{
	os << "[text: [";

	for (int i = 0 ; i < txt.getWordCount() ; ++i)
	{
		const vmime::word& w = *txt.getWordAt(i);

		if (i != 0)
			os << ",";

		os << "[word: charset=" << w.getCharset().getName() << ", buffer=" << w.getBuffer() << "]";
	}

	os << "]]";

	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::mailbox& mbox)
{
	os << "[mailbox: name=" << mbox.getName() << ", email=" << mbox.getEmail() << "]";

	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::mailboxGroup& group)
{
	os << "[mailbox-group: name=" << group.getName() << ", list=[";

	for (int i = 0 ; i < group.getMailboxCount() ; ++i)
	{
		if (i != 0)
			os << ",";

		os << *group.getMailboxAt(i);
	}

	os << "]]";

	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::addressList& list)
{
	os << "[address-list: [";

	for (int i = 0 ; i < list.getAddressCount() ; ++i)
	{
		const vmime::address& addr = *list.getAddressAt(i);

		if (i != 0)
			os << ",";

		if (addr.isGroup())
		{
			const vmime::mailboxGroup& group =
				dynamic_cast <const vmime::mailboxGroup&>(addr);

			os << group;
		}
		else
		{
			const vmime::mailbox& mbox =
				dynamic_cast <const vmime::mailbox&>(addr);

			os << mbox;
		}
	}

	os << "]]";

	return (os);
}


}
