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
		void testHas1(/*By type*/)
		{
			vmime::header hdr;
			hdr.parse("From: x\r\nTo: y\r\nSubject: test\r\n");

			bool res = hdr.fields.has(vmime::headerField::To);

			assert_eq("Value", true, res);
		}

		void testHas2(/*By type*/)
		{
			vmime::header hdr;
			hdr.parse("From: x\r\nTo: y\r\nTo: z\r\n");

			bool res = hdr.fields.has(vmime::headerField::Subject);

			assert_eq("Value", false, res);
		}

		void testHas3(/*By name*/)
		{
			vmime::header hdr;
			hdr.parse("From: x\r\nTo: y\r\nTo: z\r\n");

			bool res = hdr.fields.has("Z");

			assert_eq("Value", false, res);
		}

		void testHas4(/*By name*/)
		{
			vmime::header hdr;
			hdr.parse("X: x\r\nTo: y\r\nTo: z\r\n");

			bool res = hdr.fields.has("To");

			assert_eq("Value", true, res);
		}

		// find function tests
		void testFind1(/*By type*/)
		{
			vmime::header hdr;
			hdr.parse("From: a\r\nTo: b\r\nTo: c\r\nFrom: d\r\n");

			vmime::headerField& res = hdr.fields.find(vmime::headerField::To);

			assert_eq("Value", "To: b", getFieldValue(res));
		}

		void testFind2(/*By name*/)
		{
			vmime::header hdr;
			hdr.parse("A: a\r\nB: b\r\nC: c\r\nB: d\r\n");

			vmime::headerField& res = hdr.fields.find("B");

			assert_eq("Value", "B: b", getFieldValue(res));
		}

		// findAllByType function tests
		void testFindAllByType1()
		{
			vmime::header hdr;
			hdr.parse("To: a\r\nFrom: b\r\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByType(vmime::headerField::Subject);

			assert_eq("Count", (unsigned int) 0, res.size());
		}

		void testFindAllByType2()
		{
			vmime::header hdr;
			hdr.parse("To: b\r\nTo : a\r\nFrom: c\r\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByType(vmime::headerField::To);

			assert_eq("Count", (unsigned int) 2, res.size());
			assert_eq("First value", "To: b", getFieldValue(*res[0]));
			assert_eq("First value", "To: a", getFieldValue(*res[1]));
		}

		// findAllByName function tests
		void testFindAllByName1()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("B");

			assert_eq("Count", (unsigned int) 0, res.size());
		}

		void testFindAllByName2()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("B");

			assert_eq("Count", (unsigned int) 2, res.size());
			assert_eq("First value", "B: b1", getFieldValue(*res[0]));
			assert_eq("Second value", "B: b2", getFieldValue(*res[1]));
		}

		void testFindAllByName3()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("C");

			assert_eq("Count", (unsigned int) 3, res.size());
			assert_eq("First value", "C: c1", getFieldValue(*res[0]));
			assert_eq("Second value", "C: c3", getFieldValue(*res[1]));
			assert_eq("Second value", "C: c2", getFieldValue(*res[2]));
		}

	public:

		headerTest() : suite("vmime::header")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<my_handler>();

			add("Has", testcase(this, "Has1", &headerTest::testHas1));
			add("Has", testcase(this, "Has2", &headerTest::testHas2));
			add("Has", testcase(this, "Has3", &headerTest::testHas3));
			add("Has", testcase(this, "Has4", &headerTest::testHas4));

			add("Find", testcase(this, "Find1", &headerTest::testFind1));
			add("Find", testcase(this, "Find2", &headerTest::testFind2));

			add("FindAllByType", testcase(this, "FindAllByType1", &headerTest::testFindAllByType1));
			add("FindAllByType", testcase(this, "FindAllByType2", &headerTest::testFindAllByType2));

			add("FindAllByName", testcase(this, "FindAllByName1", &headerTest::testFindAllByName1));
			add("FindAllByName", testcase(this, "FindAllByName2", &headerTest::testFindAllByName2));
			add("FindAllByName", testcase(this, "FindAllByName3", &headerTest::testFindAllByName3));

			suite::main().add("vmime::header", this);
		}

	};

	headerTest* theTest = new headerTest();
}
