#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "../../src/vmime"
#include "../../examples/common.inc"

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

			assert_eq("Count", (unsigned int) 4, res.size());
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

			assert_eq("Count", (unsigned int) 0, res.size());
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

			assert_eq("Count", (unsigned int) 0, res.size());
		}

		void testFindAllFields2()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.findAllFields("B");

			assert_eq("Count", (unsigned int) 2, res.size());
			assert_eq("First value", "B: b1", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b2", headerTest::getFieldValue(*res[1]));
		}

		void testFindAllFields3()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

			std::vector <vmime::headerField*> res = hdr.findAllFields("C");

			assert_eq("Count", (unsigned int) 3, res.size());
			assert_eq("First value", "C: c1", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "C: c3", headerTest::getFieldValue(*res[1]));
			assert_eq("Second value", "C: c2", headerTest::getFieldValue(*res[2]));
		}

	public:

		headerTest() : suite("vmime::header")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<my_handler>();

			add("Has", testcase(this, "Has1", &headerTest::testHas1));
			add("Has", testcase(this, "Has2", &headerTest::testHas2));

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
