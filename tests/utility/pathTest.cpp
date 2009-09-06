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

#include "vmime/utility/path.hpp"


#define VMIME_TEST_SUITE         pathTest
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstruct1)
		VMIME_TEST(testConstruct2)
		VMIME_TEST(testConstruct3)
		VMIME_TEST(testConstruct4)

		VMIME_TEST(testAppendComponent)

		VMIME_TEST(testOperatorDiv1)
		VMIME_TEST(testOperatorDiv2)

		VMIME_TEST(testOperatorDivEqual1)
		VMIME_TEST(testOperatorDivEqual2)

		VMIME_TEST(testGetParent)

		VMIME_TEST(testComparison)

		VMIME_TEST(testGetLastComponent)

		VMIME_TEST(testIsDirectParentOf)
		VMIME_TEST(testIsParentOf)

		VMIME_TEST(testRenameParent)
	VMIME_TEST_LIST_END


	typedef vmime::utility::path path;
	typedef vmime::utility::path::component comp;


	void testConstruct1()
	{
		VASSERT_EQ("1", true, path().isEmpty());
		VASSERT_EQ("2", 0, path().getSize());
	}

	void testConstruct2()
	{
		path p(comp("foo"));

		VASSERT_EQ("1", false, p.isEmpty());
		VASSERT_EQ("2", 1, p.getSize());
		VASSERT_EQ("3", "foo", p.getComponentAt(0).getBuffer());
	}

	void testAppendComponent()
	{
		path p;

		VASSERT_EQ("1", 0, p.getSize());

		comp c("foo");
		p.appendComponent(c);

		VASSERT_EQ("2", 1, p.getSize());
		VASSERT_EQ("3", c.getBuffer(), p.getComponentAt(0).getBuffer());
	}

	void testConstruct3()
	{
		path p1;
		p1.appendComponent(comp("foo"));
		p1.appendComponent(comp("bar"));

		path p2(p1);

		VASSERT_EQ("1", 2, p2.getSize());
		VASSERT_EQ("2", "foo", p2.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", "bar", p2.getComponentAt(1).getBuffer());
	}

	void testConstruct4()
	{
		// Same as path::path(const component&)
		path p("foo");

		VASSERT_EQ("1", false, p.isEmpty());
		VASSERT_EQ("2", 1, p.getSize());
		VASSERT_EQ("3", "foo", p.getComponentAt(0).getBuffer());
	}

	void testOperatorDiv1()
	{
		path p1;
		p1.appendComponent(comp("foo"));
		p1.appendComponent(comp("bar"));

		path p2;
		p2.appendComponent(comp("baz"));

		path p3 = p1 / p2;

		VASSERT_EQ("1", 3, p3.getSize());
		VASSERT_EQ("2", p1.getComponentAt(0).getBuffer(), p3.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", p1.getComponentAt(1).getBuffer(), p3.getComponentAt(1).getBuffer());
		VASSERT_EQ("4", p2.getComponentAt(0).getBuffer(), p3.getComponentAt(2).getBuffer());
	}

	void testOperatorDiv2()
	{
		path p1;
		p1.appendComponent(comp("foo"));
		p1.appendComponent(comp("bar"));

		comp c("baz");

		path p2 = p1 / c;

		VASSERT_EQ("1", 3, p2.getSize());
		VASSERT_EQ("2", p1.getComponentAt(0).getBuffer(), p2.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", p1.getComponentAt(1).getBuffer(), p2.getComponentAt(1).getBuffer());
		VASSERT_EQ("4", c.getBuffer(), p2.getComponentAt(2).getBuffer());
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

		VASSERT_EQ("1", 3, p3.getSize());
		VASSERT_EQ("2", p1.getComponentAt(0).getBuffer(), p3.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", p1.getComponentAt(1).getBuffer(), p3.getComponentAt(1).getBuffer());
		VASSERT_EQ("4", p2.getComponentAt(0).getBuffer(), p3.getComponentAt(2).getBuffer());
	}

	void testOperatorDivEqual2()
	{
		path p1;
		p1.appendComponent(comp("foo"));
		p1.appendComponent(comp("bar"));

		comp c("baz");

		path p2(p1);
		p2 /= c;

		VASSERT_EQ("1", 3, p2.getSize());
		VASSERT_EQ("2", p1.getComponentAt(0).getBuffer(), p2.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", p1.getComponentAt(1).getBuffer(), p2.getComponentAt(1).getBuffer());
		VASSERT_EQ("4", c.getBuffer(), p2.getComponentAt(2).getBuffer());
	}

	void testGetParent()
	{
		path p1;
		path p1p = p1.getParent();

		VASSERT_EQ("1", true, p1p.isEmpty());

		path p2;
		p2.appendComponent(comp("foo"));
		p2.appendComponent(comp("bar"));

		path p2p = p2.getParent();

		VASSERT_EQ("2", 1, p2p.getSize());
		VASSERT_EQ("3", p2.getComponentAt(0).getBuffer(), p2p.getComponentAt(0).getBuffer());
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

		VASSERT_EQ("1", true, p1 == p2);
		VASSERT_EQ("2", false, p1 == p3);

		VASSERT_EQ("3", false, p1 != p2);
		VASSERT_EQ("4", true, p1 != p3);

		VASSERT_EQ("5", true, p3.getParent() == p1);
	}

	void testGetLastComponent()
	{
		path p1;
		p1.appendComponent(comp("foo"));
		p1.appendComponent(comp("bar"));
		p1.appendComponent(comp("baz"));

		VASSERT_EQ("1", "baz", p1.getLastComponent().getBuffer());
		VASSERT_EQ("2", "bar", p1.getParent().getLastComponent().getBuffer());
		VASSERT_EQ("3", "foo", p1.getParent().getParent().getLastComponent().getBuffer());
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

		VASSERT_EQ("1", true, p1.isDirectParentOf(p2));
		VASSERT_EQ("2", true, p2.isDirectParentOf(p3));
		VASSERT_EQ("3", false, p1.isDirectParentOf(p3));
		VASSERT_EQ("4", false, p2.isDirectParentOf(p1));
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

		VASSERT_EQ("1", true, p1.isParentOf(p2));
		VASSERT_EQ("2", true, p2.isParentOf(p3));
		VASSERT_EQ("3", true, p1.isParentOf(p3));
		VASSERT_EQ("4", false, p2.isParentOf(p1));
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

		VASSERT_EQ("1", 5, p.getSize());
		VASSERT_EQ("2", "x", p.getComponentAt(0).getBuffer());
		VASSERT_EQ("3", "y", p.getComponentAt(1).getBuffer());
		VASSERT_EQ("4", "z", p.getComponentAt(2).getBuffer());
		VASSERT_EQ("5", "c", p.getComponentAt(3).getBuffer());
		VASSERT_EQ("6", "d", p.getComponentAt(4).getBuffer());
	}

VMIME_TEST_SUITE_END

