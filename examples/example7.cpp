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
// This sample program demonstrates how to enumerate encoders and
// messaging services in VMime.
//
// For more information, please visit:
// http://www.vmime.org/
//

#include <iostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"


int main()
{
	// VMime initialization
	vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

	// Enumerate encoders
	vmime::encoderFactory* ef = vmime::encoderFactory::getInstance();

	std::cout << "Available encoders:" << std::endl;

	for (int i = 0 ; i < ef->getEncoderCount() ; ++i)
	{
		const vmime::encoderFactory::registeredEncoder& enc = *ef->getEncoderAt(i);

		std::cout << "  * " << enc.getName() << std::endl;

		vmime::encoder* e = enc.create();

		std::vector <vmime::string> props = e->getAvailableProperties();

		for (std::vector <vmime::string>::const_iterator it = props.begin() ; it != props.end() ; ++it)
			std::cout << "      - " << *it << std::endl;

		delete (e);
	}

	std::cout << std::endl;

	// Enumerate messaging services and their properties
	vmime::messaging::serviceFactory* sf = vmime::messaging::serviceFactory::getInstance();

	std::cout << "Available messaging services:" << std::endl;

	for (int i = 0 ; i < sf->getServiceCount() ; ++i)
	{
		const vmime::messaging::serviceFactory::registeredService& serv = *sf->getServiceAt(i);

		std::cout << "  * " << serv.getName() << std::endl;

		std::vector <vmime::messaging::serviceInfos::property> props =
			serv.getInfos().getAvailableProperties();

		for (std::vector <vmime::messaging::serviceInfos::property>::const_iterator it = props.begin() ;
		     it != props.end() ; ++it)
		{
			const vmime::messaging::serviceInfos::property& p = *it;

			const vmime::string name = serv.getInfos().getPropertyPrefix() + p.getName();

			vmime::string type;

			switch (p.getType())
			{
			case vmime::messaging::serviceInfos::property::TYPE_INTEGER: type = "TYPE_INTEGER"; break;
			case vmime::messaging::serviceInfos::property::TYPE_STRING: type = "TYPE_STRING"; break;
			case vmime::messaging::serviceInfos::property::TYPE_BOOL: type = "TYPE_BOOL"; break;
			default: type = "(unknown)"; break;
			}

			vmime::string flags;

			if (p.getFlags() & vmime::messaging::serviceInfos::property::FLAG_REQUIRED)
				flags += " FLAG_REQUIRED";
			if (p.getFlags() & vmime::messaging::serviceInfos::property::FLAG_HIDDEN)
				flags += " FLAG_HIDDEN";

			std::cout << "      - " << serv.getInfos().getPropertyPrefix() + p.getName();
			std::cout << " (type=" << type << ", flags=" << flags;
			std::cout << ", defaultValue=" << p.getDefaultValue() << ")" << std::endl;
		}
	}

	std::cout << std::endl;
}

