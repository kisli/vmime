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

//
// EXAMPLE DESCRIPTION:
// ====================
// This sample program demonstrate the use of the messageBuilder component
// to build a simple message with an attachment.
//
// For more information, please visit:
// http://www.vmime.org/
//

#include <iostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"


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
		mb.getTextPart()->setText(vmime::stringContentHandler(
			"I'm writing this short text to test message construction " \
			"with attachment, using the vmime::messageBuilder component."));

		// Adding an attachment
		vmime::fileAttachment* a = new vmime::fileAttachment
		(
			"./example2.cpp",                               // full path to file
			vmime::mediaType("application/octet-stream"),   // content type
			vmime::text("My first attachment")              // description
		);

		a->getFileInfo().setFilename("example2.cpp");
		a->getFileInfo().setCreationDate(vmime::datetime("30 Apr 2003 14:30:00 +0200"));

		mb.attach(a);

		// Construction
		vmime::message* msg = mb.construct();

		// Raw text generation
		vmime::string dataToSend = msg->generate();

		std::cout << "Generated message:" << std::endl;
		std::cout << "==================" << std::endl;
		std::cout << std::endl;
		std::cout << dataToSend << std::endl;

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
		throw;
	}

	std::cout << std::endl;
}

