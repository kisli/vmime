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
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

	// Enumerate encoders
	vmime::utility::encoder::encoderFactory* ef = vmime::utility::encoder::encoderFactory::getInstance();

	std::cout << "Available encoders:" << std::endl;

	for (int i = 0 ; i < ef->getEncoderCount() ; ++i)
	{
		vmime::ref <const vmime::utility::encoder::encoderFactory::registeredEncoder>
			enc = ef->getEncoderAt(i);

		std::cout << "  * " << enc->getName() << std::endl;

		vmime::ref <vmime::utility::encoder::encoder> e = enc->create();

		std::vector <vmime::string> props = e->getAvailableProperties();

		for (std::vector <vmime::string>::const_iterator it = props.begin() ; it != props.end() ; ++it)
			std::cout << "      - " << *it << std::endl;
	}

	std::cout << std::endl;

	// Enumerate messaging services and their properties
	vmime::net::serviceFactory* sf = vmime::net::serviceFactory::getInstance();

	std::cout << "Available messaging services:" << std::endl;

	for (int i = 0 ; i < sf->getServiceCount() ; ++i)
	{
		const vmime::net::serviceFactory::registeredService& serv = *sf->getServiceAt(i);

		std::cout << "  * " << serv.getName() << std::endl;

		std::vector <vmime::net::serviceInfos::property> props =
			serv.getInfos().getAvailableProperties();

		for (std::vector <vmime::net::serviceInfos::property>::const_iterator it = props.begin() ;
		     it != props.end() ; ++it)
		{
			const vmime::net::serviceInfos::property& p = *it;

			const vmime::string name = serv.getInfos().getPropertyPrefix() + p.getName();

			vmime::string type;

			switch (p.getType())
			{
			case vmime::net::serviceInfos::property::TYPE_INTEGER: type = "TYPE_INTEGER"; break;
			case vmime::net::serviceInfos::property::TYPE_STRING: type = "TYPE_STRING"; break;
			case vmime::net::serviceInfos::property::TYPE_BOOL: type = "TYPE_BOOL"; break;
			default: type = "(unknown)"; break;
			}

			vmime::string flags;

			if (p.getFlags() & vmime::net::serviceInfos::property::FLAG_REQUIRED)
				flags += " FLAG_REQUIRED";
			if (p.getFlags() & vmime::net::serviceInfos::property::FLAG_HIDDEN)
				flags += " FLAG_HIDDEN";

			std::cout << "      - " << serv.getInfos().getPropertyPrefix() + p.getName();
			std::cout << " (type=" << type << ", flags=" << flags;
			std::cout << ", defaultValue=" << p.getDefaultValue() << ")" << std::endl;
		}
	}

	std::cout << std::endl;
}

