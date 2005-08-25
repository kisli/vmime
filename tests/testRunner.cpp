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

#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"


int main(int argc, char* argv[])
{
        // VMime initialization
        vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

	// Get the top level suite from the registry
	CppUnit::Test* suite = NULL;

	if (argc > 1)
		suite = CppUnit::TestFactoryRegistry::getRegistry(argv[1]).makeTest();
	else
		suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	// Adds the test to the list of test to run
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	// Change the default outputter to a compiler error format outputter
//	runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), std::cerr));

	// Run the tests
	const bool wasSucessful = runner.run();

	// Return error code 1 if the one of test failed
	return wasSucessful ? 0 : 1;
}

