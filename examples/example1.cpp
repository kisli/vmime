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

//
// EXAMPLE DESCRIPTION:
// ====================
// This sample program demonstrate the use of the messageBuilder component
// to build a simple message.
//
// For more information, please visit:
// http://vmime.sourceforge.net/
//

#include <iostream>

#include "../src/vmime"
#include "../src/platforms/posix/handler.hpp"


int main()
{
	std::cout << std::endl;

	// VMime initialization
	vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

	try
	{
		vmime::messageBuilder mb;

		// Fill in the basic fields
		mb.setExpeditor(vmime::mailbox("me@somewhere.com"));

		vmime::addressList to;
		to.appendAddress(new vmime::mailbox("you@elsewhere.com"));

		mb.setRecipients(to);

		vmime::addressList bcc;
		bcc.appendAddress(new vmime::mailbox("you-bcc@nowhere.com"));

		mb.setBlindCopyRecipients(bcc);

		mb.setSubject(vmime::text("My first message generated with vmime::messageBuilder"));

		// Message body
		mb.getTextPart()->setText(vmime::contentHandler(
			"I'm writing this short text to test message construction " \
			"using the vmime::messageBuilder component."));

		// Construction
		vmime::message* msg = mb.construct();

		// Raw text generation
		std::cout << "Generated message:" << std::endl;
		std::cout << "==================" << std::endl;

		vmime::utility::outputStreamAdapter out(std::cout);
		msg->generate(out);

		// Destruction
		delete (msg);
	}
	// VMime exception
	catch (vmime::exception& e)
	{
		std::cout << "vmime::exception: " << e.what() << std::endl;
		throw;
	}
	// Standard exception
	catch (std::exception& e)
	{
		std::cout << "std::exception: " << e.what() << std::endl;
		//throw;
	}

	std::cout << std::endl;
}

