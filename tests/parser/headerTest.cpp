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

using namespace unitpp;


namespace
{
	class headerTest : public suite
	{
		static const std::string getFieldValue(const vmime::headerField& field)
		{
			std::ostringstream oss;
			vmime::utility::outputStreamAdapter voss(oss);
			field.generate(voss);

			return (oss.str());
		}

		// has function tests
		void testHas1()
		{
			vmime::header hdr;
			hdr.parse("From: x\r\nTo: y\r\nTo: z\r\n");

			bool res = hdr.hasField("Z");

			assert_eq("Value", false, res);
		}

		void testHas2()
		{
			vmime::header hdr;
			hdr.parse("X: x\r\nTo: y\r\nTo: z\r\n");

			bool res = hdr.hasField("To");

			assert_eq("Value", true, res);
		}

		// appendField function tests
		void testAppend1()
		{
			vmime::header hdr;
			hdr.parse("");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("A", "a");
			hdr.appendField(hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(1), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
		}

		void testAppend2()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\n");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
			hdr.appendField(hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(2), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		}

		// insertFieldBefore
		void testInsertFieldBefore1()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nC: c\r\n");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
			hdr.insertFieldBefore(hdr.getField("C"), hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(3), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b", headerTest::getFieldValue(*res[1]));
			assert_eq("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		}

		void testInsertFieldBefore2()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nC: c\r\n");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
			hdr.insertFieldBefore(1, hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(3), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b", headerTest::getFieldValue(*res[1]));
			assert_eq("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		}

		// insertFieldAfter
		void testInsertFieldAfter1()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nC: c\r\n");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
			hdr.insertFieldAfter(hdr.getField("A"), hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(3), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b", headerTest::getFieldValue(*res[1]));
			assert_eq("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		}

		void testInsertFieldAfter2()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nC: c\r\n");

			vmime::headerField* hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
			hdr.insertFieldAfter(0, hf);

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(3), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b", headerTest::getFieldValue(*res[1]));
			assert_eq("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		}

		// removeField
		void testRemoveField1()
		{
			vmime::header hdr1, hdr2;
			hdr1.parse("A: a\r\nB: b\r\nC: c\r\n");
			hdr2.parse("A: a\r\nB: b\r\nC: c\r\n");

			hdr1.removeField(hdr1.getField("B"));
			hdr2.removeField(1);

			std::vector <vmime::headerField*> res1 = hdr1.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(2), res1.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res1[0]));
			assert_eq("Second value", "C: c", headerTest::getFieldValue(*res1[1]));

			std::vector <vmime::headerField*> res2 = hdr2.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(2), res2.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res2[0]));
			assert_eq("Second value", "C: c", headerTest::getFieldValue(*res2[1]));
		}

		void testRemoveField2()
		{
			vmime::header hdr1, hdr2;
			hdr1.parse("A: a\r\n");
			hdr2.parse("A: a\r\n");

			hdr1.removeField(hdr1.getField("A"));
			hdr2.removeField(0);

			std::vector <vmime::headerField*> res1 = hdr1.getFieldList();
			assert_eq("Count", static_cast <unsigned int>(0), res1.size());

			std::vector <vmime::headerField*> res2 = hdr2.getFieldList();
			assert_eq("Count", static_cast <unsigned int>(0), res2.size());
		}

		// removeAllFields
		void testRemoveAllFields()
		{
			vmime::header hdr1, hdr2;
			hdr1.parse("A: a\r\n");
			hdr2.parse("A: a\r\nB: b\r\n");

			hdr1.removeAllFields();
			hdr2.removeAllFields();

			std::vector <vmime::headerField*> res1 = hdr1.getFieldList();
			assert_eq("Count", static_cast <unsigned int>(0), res1.size());

			std::vector <vmime::headerField*> res2 = hdr2.getFieldList();
			assert_eq("Count", static_cast <unsigned int>(0), res2.size());
		}

		// getFieldCount
		void testgetFieldCount()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nB: b\r\nC: c\r\nD: d\r\n");

			assert_eq("Value", 4, hdr.getFieldCount());
		}

		// isEmpty
		void testIsEmpty1()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nB: b\r\nC: c\r\n");

			assert_eq("Value", false, hdr.isEmpty());
		}

		void testIsEmpty2()
		{
			vmime::header hdr;
			hdr.parse("\r\n");

			assert_eq("Value", true, hdr.isEmpty());
		}

		// getFieldAt
		void getFieldAt()
		{
			vmime::header hdr;
			hdr.parse("B: b\r\nA: a\r\nC: c\r\n");

			vmime::headerField* res = hdr.getFieldAt(2);

			assert_eq("Value", "C: c", getFieldValue(*res));
		}

		// getFieldList
		void testGetFieldList1()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nB: b1\r\nC: c\r\nB: b2\r\n");

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(4), res.size());
			assert_eq("First value", "A: a", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b1", headerTest::getFieldValue(*res[1]));
			assert_eq("Third value", "C: c", headerTest::getFieldValue(*res[2]));
			assert_eq("Thourth value", "B: b2", headerTest::getFieldValue(*res[3]));
		}

		void testGetFieldList2()
		{
			vmime::header hdr;
			hdr.parse("\r\n");

			std::vector <vmime::headerField*> res = hdr.getFieldList();

			assert_eq("Count", static_cast <unsigned int>(0), res.size());
		}

		// find function tests
		void testFind1()
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nB: b\r\nC: c\r\nB: d\r\n");

			vmime::headerField* res = hdr.findField("B");

			assert_eq("Value", "B: b", getFieldValue(*res));
		}

		// getAllByName function tests
		void testFindAllFields1()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.findAllFields("B");

			assert_eq("Count", static_cast <unsigned int>(0), res.size());
		}

		void testFindAllFields2()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.findAllFields("B");

			assert_eq("Count", static_cast <unsigned int>(2), res.size());
			assert_eq("First value", "B: b1", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b2", headerTest::getFieldValue(*res[1]));
		}

		void testFindAllFields3()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

			std::vector <vmime::headerField*> res = hdr.findAllFields("C");

			assert_eq("Count", static_cast <unsigned int>(3), res.size());
			assert_eq("First value", "C: c1", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "C: c3", headerTest::getFieldValue(*res[1]));
			assert_eq("Second value", "C: c2", headerTest::getFieldValue(*res[2]));
		}

	public:

		headerTest() : suite("vmime::header")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Has", testcase(this, "Has1", &headerTest::testHas1));
			add("Has", testcase(this, "Has2", &headerTest::testHas2));

			add("Append", testcase(this, "Append1", &headerTest::testAppend1));
			add("Append", testcase(this, "Append2", &headerTest::testAppend2));

			add("InsertFieldBefore", testcase(this, "InsertFieldBefore1", &headerTest::testInsertFieldBefore1));
			add("InsertFieldBefore", testcase(this, "InsertFieldBefore2", &headerTest::testInsertFieldBefore2));

			add("InsertFieldAfter", testcase(this, "InsertFieldAfter1", &headerTest::testInsertFieldAfter1));
			add("InsertFieldAfter", testcase(this, "InsertFieldAfter2", &headerTest::testInsertFieldAfter2));

			add("RemoveField", testcase(this, "RemoveField1", &headerTest::testRemoveField1));
			add("RemoveField", testcase(this, "RemoveField2", &headerTest::testRemoveField2));

			add("RemoveAllFields", testcase(this, "RemoveAllFields", &headerTest::testRemoveAllFields));

			add("GetFieldCount", testcase(this, "GetFieldCount", &headerTest::testgetFieldCount));

			add("IsEmpty", testcase(this, "IsEmpty1", &headerTest::testIsEmpty1));
			add("IsEmpty", testcase(this, "IsEmpty2", &headerTest::testIsEmpty2));

			add("GetFieldAt", testcase(this, "GetFieldAt", &headerTest::getFieldAt));

			add("GetFieldList", testcase(this, "GetFieldList1", &headerTest::testGetFieldList1));
			add("GetFieldList", testcase(this, "GetFieldList2", &headerTest::testGetFieldList2));

			add("Find", testcase(this, "Find1", &headerTest::testFind1));

			add("FindAllFields", testcase(this, "FindAllFields1", &headerTest::testFindAllFields1));
			add("FindAllFields", testcase(this, "FindAllFields2", &headerTest::testFindAllFields2));
			add("FindAllFields", testcase(this, "FindAllFields3", &headerTest::testFindAllFields3));

			suite::main().add("vmime::header", this);
		}

	};

	headerTest* theTest = new headerTest();
}
