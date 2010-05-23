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

//
// EXAMPLE DESCRIPTION:
// ====================
// This sample program demonstrate the use of the messageBuilder component
// to build a complex message (HTML content, plain text and embedded image).
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
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

	try
	{
		vmime::messageBuilder mb;

		// Fill in the basic fields
		mb.setExpeditor(vmime::mailbox("me@somewhere.com"));

		vmime::addressList to;
		to.appendAddress(vmime::create <vmime::mailbox>("you@elsewhere.com"));

		mb.setRecipients(to);

		vmime::addressList bcc;
		bcc.appendAddress(vmime::create <vmime::mailbox>("you-bcc@nowhere.com"));

		mb.setBlindCopyRecipients(bcc);

		mb.setSubject(vmime::text("My first message generated with vmime::messageBuilder"));

		// Set the content-type to "text/html"
		mb.constructTextPart(vmime::mediaType
			(vmime::mediaTypes::TEXT, vmime::mediaTypes::TEXT_HTML));

		// Fill in the text part: the message is available in two formats: HTML and plain text.
		// HTML text part also includes an inline image (embedded into the message).
		vmime::htmlTextPart& textPart = *mb.getTextPart().dynamicCast <vmime::htmlTextPart>();

		// -- embed an image (the returned "CID" (content identifier) is used to reference
		// -- the image into HTML content).
		vmime::ref <vmime::utility::fileSystemFactory> fs =
			vmime::platform::getHandler()->getFileSystemFactory();

		vmime::ref <vmime::utility::file> imageFile =
			fs->create(fs->stringToPath("/path/to/image.jpg"));

		vmime::ref <vmime::utility::fileReader> fileReader =
			imageFile->getFileReader();

		vmime::ref <vmime::contentHandler> imageCts =
			vmime::create <vmime::streamContentHandler>
				(fileReader->getInputStream(), imageFile->getLength());

		const vmime::string cid = textPart.addObject(imageCts,
			vmime::mediaType(vmime::mediaTypes::IMAGE, vmime::mediaTypes::IMAGE_JPEG));

		// -- message text
		textPart.setText(vmime::create <vmime::stringContentHandler>
			(vmime::string("This is the <b>HTML text</b>.<br/><img src=\"") + cid + vmime::string("\"/>")));
		textPart.setPlainText(vmime::create <vmime::stringContentHandler>
			("This is the plain text (without HTML formatting)."));

		// Construction
		vmime::ref <vmime::message> msg = mb.construct();

		// Raw text generation
		vmime::string dataToSend = msg->generate();

		std::cout << "Generated message:" << std::endl;
		std::cout << "==================" << std::endl;
		std::cout << std::endl;
		std::cout << dataToSend << std::endl;
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

