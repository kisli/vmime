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

			add("FindAllFields", testcase(this, "FindAllFields1", &headerTest::testFindAllFields1));
			add("FindAllFields", testcase(this, "FindAllFields2", &headerTest::testFindAllFields2));
			add("FindAllFields", testcase(this, "FindAllFields3", &headerTest::testFindAllFields3));
			suite::main().add("vmime::header", this);
		}

	};

	headerTest* theTest = new headerTest();
}
