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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"
#include "vmime/utility/path.hpp"

using namespace unitpp;


namespace
{
	class pathTest : public suite
	{
		typedef vmime::utility::path path;
		typedef vmime::utility::path::component comp;


		void testConstruct1()
		{
			assert_eq("1", true, path().isEmpty());
			assert_eq("2", 0, path().getSize());
		}

		void testConstruct2()
		{
			path p(comp("foo"));

			assert_eq("1", false, p.isEmpty());
			assert_eq("2", 1, p.getSize());
			assert_eq("3", "foo", p.getComponentAt(0).getBuffer());
		}

		void testAppendComponent()
		{
			path p;

			assert_eq("1", 0, p.getSize());

			comp c("foo");
			p.appendComponent(c);

			assert_eq("2", 1, p.getSize());
			assert_eq("3", c.getBuffer(), p.getComponentAt(0).getBuffer());
		}

		void testConstruct3()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			path p2(p1);

			assert_eq("1", 2, p2.getSize());
			assert_eq("2", "foo", p2.getComponentAt(0).getBuffer());
			assert_eq("3", "bar", p2.getComponentAt(1).getBuffer());
		}

		void testConstruct4()
		{
			// Same as path::path(const component&)
			path p("foo");

			assert_eq("1", false, p.isEmpty());
			assert_eq("2", 1, p.getSize());
			assert_eq("3", "foo", p.getComponentAt(0).getBuffer());
		}

		void testOperatorDiv1()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			path p2;
			p2.appendComponent(comp("baz"));

			path p3 = p1 / p2;

			assert_eq("1", 3, p3.getSize());
			assert_eq("2", p1.getComponentAt(0).getBuffer(), p3.getComponentAt(0).getBuffer());
			assert_eq("3", p1.getComponentAt(1).getBuffer(), p3.getComponentAt(1).getBuffer());
			assert_eq("4", p2.getComponentAt(0).getBuffer(), p3.getComponentAt(2).getBuffer());
		}

		void testOperatorDiv2()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			comp c("baz");

			path p2 = p1 / c;

			assert_eq("1", 3, p2.getSize());
			assert_eq("2", p1.getComponentAt(0).getBuffer(), p2.getComponentAt(0).getBuffer());
			assert_eq("3", p1.getComponentAt(1).getBuffer(), p2.getComponentAt(1).getBuffer());
			assert_eq("4", c.getBuffer(), p2.getComponentAt(2).getBuffer());
		}

		void testOperatorDivEqual1()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			path p2;
			p2.appendComponent(comp("baz"));

			path p3(p1);
			p3 /= p2;

			assert_eq("1", 3, p3.getSize());
			assert_eq("2", p1.getComponentAt(0).getBuffer(), p3.getComponentAt(0).getBuffer());
			assert_eq("3", p1.getComponentAt(1).getBuffer(), p3.getComponentAt(1).getBuffer());
			assert_eq("4", p2.getComponentAt(0).getBuffer(), p3.getComponentAt(2).getBuffer());
		}

		void testOperatorDivEqual2()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			comp c("baz");

			path p2(p1);
			p2 /= c;

			assert_eq("1", 3, p2.getSize());
			assert_eq("2", p1.getComponentAt(0).getBuffer(), p2.getComponentAt(0).getBuffer());
			assert_eq("3", p1.getComponentAt(1).getBuffer(), p2.getComponentAt(1).getBuffer());
			assert_eq("4", c.getBuffer(), p2.getComponentAt(2).getBuffer());
		}

		void testGetParent()
		{
			path p1;
			path p1p = p1.getParent();

			assert_eq("1", true, p1p.isEmpty());

			path p2;
			p2.appendComponent(comp("foo"));
			p2.appendComponent(comp("bar"));

			path p2p = p2.getParent();

			assert_eq("2", 1, p2p.getSize());
			assert_eq("3", p2.getComponentAt(0).getBuffer(), p2p.getComponentAt(0).getBuffer());
		}

		void testComparison()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));

			path p2;
			p2.appendComponent(comp("foo"));
			p2.appendComponent(comp("bar"));

			path p3;
			p3.appendComponent(comp("foo"));
			p3.appendComponent(comp("bar"));
			p3.appendComponent(comp("baz"));

			assert_eq("1", true, p1 == p2);
			assert_eq("2", false, p1 == p3);

			assert_eq("3", false, p1 != p2);
			assert_eq("4", true, p1 != p3);

			assert_eq("5", true, p3.getParent() == p1);
		}

		void testGetLastComponent()
		{
			path p1;
			p1.appendComponent(comp("foo"));
			p1.appendComponent(comp("bar"));
			p1.appendComponent(comp("baz"));

			assert_eq("1", "baz", p1.getLastComponent().getBuffer());
			assert_eq("2", "bar", p1.getParent().getLastComponent().getBuffer());
			assert_eq("3", "foo", p1.getParent().getParent().getLastComponent().getBuffer());
		}

		void testIsDirectParentOf()
		{
			path p1;
			p1.appendComponent(comp("foo"));

			path p2;
			p2.appendComponent(comp("foo"));
			p2.appendComponent(comp("bar"));

			path p3;
			p3.appendComponent(comp("foo"));
			p3.appendComponent(comp("bar"));
			p3.appendComponent(comp("baz"));

			assert_eq("1", true, p1.isDirectParentOf(p2));
			assert_eq("2", true, p2.isDirectParentOf(p3));
			assert_eq("3", false, p1.isDirectParentOf(p3));
			assert_eq("4", false, p2.isDirectParentOf(p1));
		}

		void testIsParentOf()
		{
			path p1;
			p1.appendComponent(comp("foo"));

			path p2;
			p2.appendComponent(comp("foo"));
			p2.appendComponent(comp("bar"));

			path p3;
			p3.appendComponent(comp("foo"));
			p3.appendComponent(comp("bar"));
			p3.appendComponent(comp("baz"));

			assert_eq("1", true, p1.isParentOf(p2));
			assert_eq("2", true, p2.isParentOf(p3));
			assert_eq("3", true, p1.isParentOf(p3));
			assert_eq("4", false, p2.isParentOf(p1));
		}

		void testRenameParent()
		{
			path p1;
			p1.appendComponent(comp("a"));
			p1.appendComponent(comp("b"));
			p1.appendComponent(comp("c"));
			p1.appendComponent(comp("d"));

			path p2;
			p2.appendComponent(comp("a"));
			p2.appendComponent(comp("b"));

			path p3;
			p3.appendComponent(comp("x"));
			p3.appendComponent(comp("y"));
			p3.appendComponent(comp("z"));

			path p(p1);
			p.renameParent(p2, p3);

			assert_eq("1", 5, p.getSize());
			assert_eq("2", "x", p.getComponentAt(0).getBuffer());
			assert_eq("3", "y", p.getComponentAt(1).getBuffer());
			assert_eq("4", "z", p.getComponentAt(2).getBuffer());
			assert_eq("5", "c", p.getComponentAt(3).getBuffer());
			assert_eq("6", "d", p.getComponentAt(4).getBuffer());
		}

	public:

		pathTest() : suite("vmime::utility::path")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Construct1", testcase(this, "Construct1", &pathTest::testConstruct1));
			add("Construct2", testcase(this, "Construct2", &pathTest::testConstruct2));
			add("Construct3", testcase(this, "Construct3", &pathTest::testConstruct3));
			add("Construct4", testcase(this, "Construct4", &pathTest::testConstruct4));

			add("AppendComponent", testcase(this, "AppendComponent", &pathTest::testAppendComponent));

			add("OperatorDiv1", testcase(this, "OperatorDiv1", &pathTest::testOperatorDiv1));
			add("OperatorDiv2", testcase(this, "OperatorDiv2", &pathTest::testOperatorDiv2));

			add("OperatorDivEqual1", testcase(this, "OperatorDivEqual1", &pathTest::testOperatorDivEqual1));
			add("OperatorDivEqual2", testcase(this, "OperatorDivEqual2", &pathTest::testOperatorDivEqual2));

			add("GetParent", testcase(this, "GetParent", &pathTest::testGetParent));

			add("Comparison", testcase(this, "Comparison", &pathTest::testComparison));

			add("GetLastComponent", testcase(this, "GetLastComponent", &pathTest::testGetLastComponent));

			add("IsDirectParentOf", testcase(this, "IsDirectParentOf", &pathTest::testIsDirectParentOf));
			add("IsParentOf", testcase(this, "IsParentOf", &pathTest::testIsParentOf));

			add("RenameParent", testcase(this, "RenameParent", &pathTest::testRenameParent));

			suite::main().add("vmime::utility::path", this);
		}

	};

	pathTest* theTest = new pathTest();
}
