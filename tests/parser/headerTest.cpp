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

			add("Find", testcase(this, "Find1", &headerTest::testFind1));

			add("FindAllFields", testcase(this, "FindAllFields1", &headerTest::testFindAllFields1));
			add("FindAllFields", testcase(this, "FindAllFields2", &headerTest::testFindAllFields2));
			add("FindAllFields", testcase(this, "FindAllFields3", &headerTest::testFindAllFields3));

			suite::main().add("vmime::header", this);
		}

	};

	headerTest* theTest = new headerTest();
}
