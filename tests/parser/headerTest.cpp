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

		// getAllByType function tests
		void testGetAllByType1()
		{
			vmime::header hdr;
			hdr.parse("To: a\r\nFrom: b\r\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByType(vmime::headerField::Subject);

			assert_eq("Count", (unsigned int) 0, res.size());
		}

		void testGetAllByType2()
		{
			vmime::header hdr;
			hdr.parse("To: b\r\nTo : a\r\nFrom: c\r\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByType(vmime::headerField::To);

			assert_eq("Count", (unsigned int) 2, res.size());
			assert_eq("First value", "To: b", headerTest::getFieldValue(*res[0]));
			assert_eq("First value", "To: a", headerTest::getFieldValue(*res[1]));
		}

		// getAllByName function tests
		void testGetAllByName1()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("B");

			assert_eq("Count", (unsigned int) 0, res.size());
		}

		void testGetAllByName2()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("B");

			assert_eq("Count", (unsigned int) 2, res.size());
			assert_eq("First value", "B: b1", headerTest::getFieldValue(*res[0]));
			assert_eq("Second value", "B: b2", headerTest::getFieldValue(*res[1]));
		}

		void testGetAllByName3()
		{
			vmime::header hdr;
			hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

			std::vector <vmime::headerField*> res = hdr.fields.findAllByName("C");

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

			add("GetAllByType", testcase(this, "GetAllByType1", &headerTest::testGetAllByType1));
			add("GetAllByType", testcase(this, "GetAllByType2", &headerTest::testGetAllByType2));
			add("GetAllByName", testcase(this, "GetAllByName1", &headerTest::testGetAllByName1));
			add("GetAllByName", testcase(this, "GetAllByName2", &headerTest::testGetAllByName2));
			add("GetAllByName", testcase(this, "GetAllByName3", &headerTest::testGetAllByName3));
			suite::main().add("vmime::header", this);
		}

	};

	headerTest* theTest = new headerTest();
}
