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
// This sample program demonstrate the use of the messageParser component
// to enumerate the attachments in a message.
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
		vmime::messageParser mp("<...MIME message content...>");

		// Enumerate attachments
		for (int i = 0 ; i < mp.getAttachmentCount() ; ++i)
		{
			const vmime::attachment& att = *mp.getAttachmentAt(i);

			// Media type (content type) is in "att.getType()"
			// Description is in "att.getDescription()"
			// Data is in "att.getData()"
		}
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
