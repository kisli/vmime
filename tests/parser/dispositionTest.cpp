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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	class dispositionTest : public suite
	{
		void testParse()
		{
			// disposition-mode ";" disposition-type
			//      [ "/" disposition-modifier *( "," disposition-modifier ) ]
			//
			// disposition-mode = action-mode "/" sending-mode

			vmime::disposition disp1;
			disp1.parse("mode");

			assert_eq("1.1", "mode", disp1.getActionMode());
			assert_eq("1.2", "", disp1.getSendingMode());
			assert_eq("1.3", "", disp1.getType());
			assert_eq("1.4", 0, static_cast <int>(disp1.getModifierList().size()));

			vmime::disposition disp2;
			disp2.parse("amode/smode");

			assert_eq("2.1", "amode", disp2.getActionMode());
			assert_eq("2.2", "smode", disp2.getSendingMode());
			assert_eq("2.3", "", disp2.getType());
			assert_eq("2.4", 0, static_cast <int>(disp2.getModifierList().size()));

			vmime::disposition disp3;
			disp3.parse("amode/smode;type");

			assert_eq("3.1", "amode", disp3.getActionMode());
			assert_eq("3.2", "smode", disp3.getSendingMode());
			assert_eq("3.3", "type", disp3.getType());
			assert_eq("3.4", 0, static_cast <int>(disp3.getModifierList().size()));

			vmime::disposition disp4;
			disp4.parse("amode/smode;type/modif");

			assert_eq("4.1", "amode", disp4.getActionMode());
			assert_eq("4.2", "smode", disp4.getSendingMode());
			assert_eq("4.3", "type", disp4.getType());
			assert_eq("4.4", 1, static_cast <int>(disp4.getModifierList().size()));
			assert_eq("4.5", "modif", disp4.getModifierList()[0]);

			vmime::disposition disp5;
			disp5.parse("amode/smode;type/modif1,modif2");

			assert_eq("5.1", "amode", disp5.getActionMode());
			assert_eq("5.2", "smode", disp5.getSendingMode());
			assert_eq("5.3", "type", disp5.getType());
			assert_eq("5.4", 2, static_cast <int>(disp5.getModifierList().size()));
			assert_eq("5.5", "modif1", disp5.getModifierList()[0]);
			assert_eq("5.6", "modif2", disp5.getModifierList()[1]);
		}

		void testGenerate()
		{
			vmime::disposition disp;

			assert_eq("1", "automatic-action/MDN-sent-automatically;displayed", disp.generate());

			disp.setActionMode("amode");

			assert_eq("2", "amode/MDN-sent-automatically;displayed", disp.generate());

			disp.setActionMode("amode");
			disp.setSendingMode("smode");

			assert_eq("3", "amode/smode;displayed", disp.generate());

			disp.setType("type");

			assert_eq("4", "amode/smode;type", disp.generate());

			disp.addModifier("modif1");

			assert_eq("5", "amode/smode;type/modif1", disp.generate());

			disp.addModifier("modif2");

			assert_eq("6", "amode/smode;type/modif1,modif2", disp.generate());
		}

		void testModifiers()
		{
			vmime::disposition disp1;

			assert_eq("1", false, disp1.hasModifier("foo"));
			assert_eq("2", 0, static_cast <int>(disp1.getModifierList().size()));

			disp1.addModifier("bar");

			assert_eq("3", false, disp1.hasModifier("foo"));
			assert_eq("4", true, disp1.hasModifier("bar"));
			assert_eq("5", 1, static_cast <int>(disp1.getModifierList().size()));

			disp1.addModifier("plop");

			assert_eq("6", false, disp1.hasModifier("foo"));
			assert_eq("7", true, disp1.hasModifier("bar"));
			assert_eq("8", true, disp1.hasModifier("plop"));
			assert_eq("9", 2, static_cast <int>(disp1.getModifierList().size()));

			disp1.removeModifier("bar");

			assert_eq("10", false, disp1.hasModifier("foo"));
			assert_eq("11", false, disp1.hasModifier("bar"));
			assert_eq("12", true, disp1.hasModifier("plop"));
			assert_eq("13", 1, static_cast <int>(disp1.getModifierList().size()));

			disp1.removeModifier("PlOp");

			assert_eq("14", false, disp1.hasModifier("foo"));
			assert_eq("15", false, disp1.hasModifier("bar"));
			assert_eq("16", false, disp1.hasModifier("plop"));
			assert_eq("17", 0, static_cast <int>(disp1.getModifierList().size()));
		}

	public:

		dispositionTest() : suite("vmime::disposition")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &dispositionTest::testParse));
			add("Generate", testcase(this, "Generate", &dispositionTest::testGenerate));

			add("Modifiers", testcase(this, "Modifiers", &dispositionTest::testModifiers));

			suite::main().add("vmime::disposition", this);
		}

	};

	dispositionTest* theTest = new dispositionTest();
}
