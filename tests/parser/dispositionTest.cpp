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

#include "tests/testUtils.hpp"


#define VMIME_TEST_SUITE         dispositionTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testModifiers)
	VMIME_TEST_LIST_END


	void testParse()
	{
		// disposition-mode ";" disposition-type
		//      [ "/" disposition-modifier *( "," disposition-modifier ) ]
		//
		// disposition-mode = action-mode "/" sending-mode

		vmime::disposition disp1;
		disp1.parse("mode");

		VASSERT_EQ("1.1", "mode", disp1.getActionMode());
		VASSERT_EQ("1.2", "", disp1.getSendingMode());
		VASSERT_EQ("1.3", "", disp1.getType());
		VASSERT_EQ("1.4", 0, static_cast <int>(disp1.getModifierList().size()));

		vmime::disposition disp2;
		disp2.parse("amode/smode");

		VASSERT_EQ("2.1", "amode", disp2.getActionMode());
		VASSERT_EQ("2.2", "smode", disp2.getSendingMode());
		VASSERT_EQ("2.3", "", disp2.getType());
		VASSERT_EQ("2.4", 0, static_cast <int>(disp2.getModifierList().size()));

		vmime::disposition disp3;
		disp3.parse("amode/smode;type");

		VASSERT_EQ("3.1", "amode", disp3.getActionMode());
		VASSERT_EQ("3.2", "smode", disp3.getSendingMode());
		VASSERT_EQ("3.3", "type", disp3.getType());
		VASSERT_EQ("3.4", 0, static_cast <int>(disp3.getModifierList().size()));

		vmime::disposition disp4;
		disp4.parse("amode/smode;type/modif");

		VASSERT_EQ("4.1", "amode", disp4.getActionMode());
		VASSERT_EQ("4.2", "smode", disp4.getSendingMode());
		VASSERT_EQ("4.3", "type", disp4.getType());
		VASSERT_EQ("4.4", 1, static_cast <int>(disp4.getModifierList().size()));
		VASSERT_EQ("4.5", "modif", disp4.getModifierList()[0]);

		vmime::disposition disp5;
		disp5.parse("amode/smode;type/modif1,modif2");

		VASSERT_EQ("5.1", "amode", disp5.getActionMode());
		VASSERT_EQ("5.2", "smode", disp5.getSendingMode());
		VASSERT_EQ("5.3", "type", disp5.getType());
		VASSERT_EQ("5.4", 2, static_cast <int>(disp5.getModifierList().size()));
		VASSERT_EQ("5.5", "modif1", disp5.getModifierList()[0]);
		VASSERT_EQ("5.6", "modif2", disp5.getModifierList()[1]);
	}

	void testGenerate()
	{
		vmime::disposition disp;

		VASSERT_EQ("1", "automatic-action/MDN-sent-automatically;displayed", disp.generate());

		disp.setActionMode("amode");

		VASSERT_EQ("2", "amode/MDN-sent-automatically;displayed", disp.generate());

		disp.setActionMode("amode");
		disp.setSendingMode("smode");

		VASSERT_EQ("3", "amode/smode;displayed", disp.generate());

		disp.setType("type");

		VASSERT_EQ("4", "amode/smode;type", disp.generate());

		disp.addModifier("modif1");

		VASSERT_EQ("5", "amode/smode;type/modif1", disp.generate());

		disp.addModifier("modif2");

		VASSERT_EQ("6", "amode/smode;type/modif1,modif2", disp.generate());
	}

	void testModifiers()
	{
		vmime::disposition disp1;

		VASSERT_EQ("1", false, disp1.hasModifier("foo"));
		VASSERT_EQ("2", 0, static_cast <int>(disp1.getModifierList().size()));

		disp1.addModifier("bar");

		VASSERT_EQ("3", false, disp1.hasModifier("foo"));
		VASSERT_EQ("4", true, disp1.hasModifier("bar"));
		VASSERT_EQ("5", 1, static_cast <int>(disp1.getModifierList().size()));

		disp1.addModifier("plop");

		VASSERT_EQ("6", false, disp1.hasModifier("foo"));
		VASSERT_EQ("7", true, disp1.hasModifier("bar"));
		VASSERT_EQ("8", true, disp1.hasModifier("plop"));
		VASSERT_EQ("9", 2, static_cast <int>(disp1.getModifierList().size()));

		disp1.removeModifier("bar");

		VASSERT_EQ("10", false, disp1.hasModifier("foo"));
		VASSERT_EQ("11", false, disp1.hasModifier("bar"));
		VASSERT_EQ("12", true, disp1.hasModifier("plop"));
		VASSERT_EQ("13", 1, static_cast <int>(disp1.getModifierList().size()));

		disp1.removeModifier("PlOp");

		VASSERT_EQ("14", false, disp1.hasModifier("foo"));
		VASSERT_EQ("15", false, disp1.hasModifier("bar"));
		VASSERT_EQ("16", false, disp1.hasModifier("plop"));
		VASSERT_EQ("17", 0, static_cast <int>(disp1.getModifierList().size()));
	}

VMIME_TEST_SUITE_END

