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

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <fstream>

#include "../../src/vmime"
#include "../../examples/common.inc"



std::ostream& operator<<(std::ostream& os, const vmime::text& txt)
{
	os << "[";

	for (int i = 0 ; i < txt.getWordCount() ; ++i)
	{
		const vmime::word& w = *txt.getWordAt(i);

		if (i != 0)
			os << ",";

		os << "[" << w.getCharset().getName() << "," << w.getBuffer() << "]";
	}

	os << "]";

	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::mailbox& mbox)
{
	std::cout << "MAILBOX[name=" << mbox.getName() << ",email=" << mbox.getEmail() << "]" << std::endl;

	return (os);
}


std::ostream& operator<<(std::ostream& os, const vmime::mailboxGroup& group)
{
	std::cout << "GROUP[name=" << group.getName() << "]" << std::endl;

	for (int i = 0 ; i < group.getMailboxCount() ; ++i)
	{
		std::cout << "* " << *group.getMailboxAt(i);
	}

	return (os);
}


int main(int argc, char* argv[])
{
	// VMime initialization
	vmime::platformDependant::setHandler<my_handler>();


	// Read data from standard input
	std::ostringstream data;
	std::istream* input = &std::cin;
	std::ifstream file;

	if (argc >= 2)
	{
		file.open(argv[1], std::ios::in | std::ios::binary);
		input = &file;
	}

	while (!input->eof())
	{
		char buffer[4096];
		input->read(buffer, sizeof(buffer));
		data.write(buffer, input->gcount());
	}

	// Parse address list and output results
	vmime::addressList list;
	list.parse(data.str());

	for (int i = 0 ; i < list.getAddressCount() ; ++i)
	{
		const vmime::address& addr = *list.getAddressAt(i);

		if (addr.isGroup())
		{
			const vmime::mailboxGroup& group =
				dynamic_cast <const vmime::mailboxGroup&>(addr);

			std::cout << group;
		}
		else
		{
			const vmime::mailbox& mbox =
				dynamic_cast <const vmime::mailbox&>(addr);

			std::cout << mbox;
		}
	}
}
