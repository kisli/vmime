//
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
// to enumerate the text parts in a message.
//
// For more information, please visit:
// http://vmime.sourceforge.net/
//

#include <iostream>

#include "../src/vmime"
#include "common.inc"


int main()
{
	std::cout << std::endl;

	// VMime initialization
	vmime::platformDependant::setHandler<my_handler>();

	try
	{
		vmime::messageParser mp("<...MIME message content...>");

		// Enumerate text parts
		for (std::vector <vmime::textPart*>::const_iterator i = mp.textParts().begin() ;
		     i != mp.textParts().end() ; ++i)
		{
			const vmime::textPart& part = **i;

			// Output content-type of the part
			std::cout << part.type().generate() << std::endl;

			// text/html
			if (part.type().subType() == vmime::mediaTypes::TEXT_HTML)
			{
				const vmime::htmlTextPart& hp = dynamic_cast<const vmime::htmlTextPart&>(part);

				// HTML text is in "hp.text()"
				// Corresponding plain text is in "hp.plainText()"

				// Enumerate embedded objects (eg. images)
				for (vmime::htmlTextPart::const_iterator i = hp.embeddedObjects.begin() ;
				     i != hp.embeddedObjects.end() ; ++i)
				{
					// Identifier (content-id or content-location) is in "(*i).id()"
					// Object data is in "(*i).data()"
				}
			}
			// text/plain
			else
			{
				const vmime::textPart& tp = dynamic_cast<const vmime::textPart&>(part);

				// Text is in "tp.text()"
			}
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
