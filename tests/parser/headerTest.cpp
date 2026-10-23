//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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


VMIME_TEST_SUITE_BEGIN(headerTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testHas1)
		VMIME_TEST(testHas2)

		VMIME_TEST(testAppend1)
		VMIME_TEST(testAppend2)

		VMIME_TEST(testInsertFieldBefore1)
		VMIME_TEST(testInsertFieldBefore2)

		VMIME_TEST(testInsertFieldAfter1)
		VMIME_TEST(testInsertFieldAfter2)

		VMIME_TEST(testReplaceField)

		VMIME_TEST(testRemoveField1)
		VMIME_TEST(testRemoveField2)

		VMIME_TEST(testRemoveAllFields)

		VMIME_TEST(testgetFieldCount)

		VMIME_TEST(testIsEmpty1)
		VMIME_TEST(testIsEmpty2)

		VMIME_TEST(testGetFieldAt)

		VMIME_TEST(testGetFieldList1)
		VMIME_TEST(testGetFieldList2)

		VMIME_TEST(testFind1)

		VMIME_TEST(testFindAllFields1)
		VMIME_TEST(testFindAllFields2)
		VMIME_TEST(testFindAllFields3)

		VMIME_TEST(testParseFromStream)
	VMIME_TEST_LIST_END


	static const std::string getFieldValue(const vmime::headerField& field) {

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter voss(oss);
		field.generate(voss);

		return oss.str();
	}

	// has function tests
	void testHas1() {

		vmime::header hdr;
		hdr.parse("From: x\r\nTo: y\r\nTo: z\r\n");

		bool res = hdr.hasField("Z");

		VASSERT_EQ("Value", false, res);
	}

	void testHas2() {

		vmime::header hdr;
		hdr.parse("X: x\r\nTo: y\r\nTo: z\r\n");

		bool res = hdr.hasField("To");

		VASSERT_EQ("Value", true, res);
	}

	// appendField function tests
	void testAppend1() {

		vmime::header hdr;
		hdr.parse("");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("A", "a");
		hdr.appendField(hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(1), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
	}

	void testAppend2() {

		vmime::header hdr;
		hdr.parse("A: a\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.appendField(hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
	}

	// insertFieldBefore
	void testInsertFieldBefore1() {

		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldBefore(hdr.getField("C"), hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	void testInsertFieldBefore2() {

		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldBefore(1, hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	// insertFieldAfter
	void testInsertFieldAfter1() {

		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldAfter(hdr.getField("A"), hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	void testInsertFieldAfter2() {

		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldAfter(0, hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	// replaceField
	void testReplaceField() {

		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("Z", "z");
		hdr.replaceField(hdr.getField("B"), hf);

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "Z: z", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	// removeField
	void testRemoveField1() {

		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\nB: b\r\nC: c\r\n");
		hdr2.parse("A: a\r\nB: b\r\nC: c\r\n");

		hdr1.removeField(hdr1.getField("B"));
		hdr2.removeField(1);

		std::vector <vmime::shared_ptr <vmime::headerField> > res1 = hdr1.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res1.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res1[0]));
		VASSERT_EQ("Second value", "C: c", headerTest::getFieldValue(*res1[1]));

		std::vector <vmime::shared_ptr <vmime::headerField> > res2 = hdr2.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res2.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res2[0]));
		VASSERT_EQ("Second value", "C: c", headerTest::getFieldValue(*res2[1]));
	}

	void testRemoveField2() {

		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\n");
		hdr2.parse("A: a\r\n");

		hdr1.removeField(hdr1.getField("A"));
		hdr2.removeField(0);

		std::vector <vmime::shared_ptr <vmime::headerField> > res1 = hdr1.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res1.size());

		std::vector <vmime::shared_ptr <vmime::headerField> > res2 = hdr2.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res2.size());
	}

	// removeAllFields
	void testRemoveAllFields() {

		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\n");
		hdr2.parse("A: a\r\nB: b\r\n");

		hdr1.removeAllFields();
		hdr2.removeAllFields();

		std::vector <vmime::shared_ptr <vmime::headerField> > res1 = hdr1.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res1.size());

		std::vector <vmime::shared_ptr <vmime::headerField> > res2 = hdr2.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res2.size());
	}

	// getFieldCount
	void testgetFieldCount() {

		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\nD: d\r\n");

		VASSERT_EQ("Value", 4, hdr.getFieldCount());
	}

	// isEmpty
	void testIsEmpty1() {

		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\n");

		VASSERT_EQ("Value", false, hdr.isEmpty());
	}

	void testIsEmpty2() {

		vmime::header hdr;
		hdr.parse("\r\n");

		VASSERT_EQ("Value", true, hdr.isEmpty());
	}

	// getFieldAt
	void testGetFieldAt() {

		vmime::header hdr;
		hdr.parse("B: b\r\nA: a\r\nC: c\r\n");

		vmime::shared_ptr <vmime::headerField> res = hdr.getFieldAt(2);

		VASSERT_EQ("Value", "C: c", getFieldValue(*res));
	}

	// getFieldList
	void testGetFieldList1() {

		vmime::header hdr;
		hdr.parse("A: a\r\nB: b1\r\nC: c\r\nB: b2\r\n");

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(4), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b1", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		VASSERT_EQ("Thourth value", "B: b2", headerTest::getFieldValue(*res[3]));
	}

	void testGetFieldList2() {

		vmime::header hdr;
		hdr.parse("\r\n");

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(0), res.size());
	}

	// find function tests
	void testFind1() {

		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\nB: d\r\n");

		vmime::shared_ptr <vmime::headerField> res = hdr.findField("B");

		VASSERT_EQ("Value", "B: b", getFieldValue(*res));
	}

	// getAllByName function tests
	void testFindAllFields1() {

		vmime::header hdr;
		hdr.parse("A: a1\nC: c1\n");

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.findAllFields("B");

		VASSERT_EQ("Count", static_cast <unsigned int>(0), res.size());
	}

	void testFindAllFields2() {

		vmime::header hdr;
		hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.findAllFields("B");

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res.size());
		VASSERT_EQ("First value", "B: b1", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b2", headerTest::getFieldValue(*res[1]));
	}

	void testFindAllFields3() {

		vmime::header hdr;
		hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

		std::vector <vmime::shared_ptr <vmime::headerField> > res = hdr.findAllFields("C");

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "C: c1", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "C: c3", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Second value", "C: c2", headerTest::getFieldValue(*res[2]));
	}

	void testParseFromStream() {

		// When parsing from a stream, the header is only extracted up to the
		// first empty line: results must be the same as from a string
		struct TestCase {
			const char* data;
			vmime::headerParseRecoveryMethod::headerLineError recovery;
			size_t fieldCount;
			size_t end;
		};

		static const vmime::headerParseRecoveryMethod::headerLineError SKIP_LINE =
			vmime::headerParseRecoveryMethod::SKIP_LINE;
		static const vmime::headerParseRecoveryMethod::headerLineError ASSUME_END_OF_HEADERS =
			vmime::headerParseRecoveryMethod::ASSUME_END_OF_HEADERS;

		static const TestCase testCases[] = {
			{ "A: a\r\nB: b\r\n\r\nBODY: x\r\n\r\n", SKIP_LINE, 2, 14 },
			{ "A: a\nB: b\n\nBODY: x\n\n", SKIP_LINE, 2, 11 },
			{ "A: a\r\nB: b\r\n", SKIP_LINE, 2, 12 },  // no empty line
			{ "\r\nBODY: x\r\n", SKIP_LINE, 0, 2 },
			{ "\nBODY: x\n", SKIP_LINE, 0, 1 },
			{ "A: a\r\n \r\nB: b\r\n\r\nBODY: x\r\n", SKIP_LINE, 2, 17 },  // folding
			{ "A: a\r\n\tfolded\r\n\r\nBODY: x\r\n", SKIP_LINE, 1, 17 },
			{ "A: a\r\n\r\r\nB: b\r\n\r\nBODY: x\r\n", SKIP_LINE, 2, 17 },  // not empty
			{ " lead\r\nA: a\r\n\r\nBODY: x\r\n", SKIP_LINE, 1, 15 },
			{ "A: a\r\ngarbage\r\nB: b\r\n\r\nBODY: x\r\n", SKIP_LINE, 2, 23 },
			{ "A: a\r\ngarbage\r\nB: b\r\n\r\nBODY: x\r\n", ASSUME_END_OF_HEADERS, 1, 6 },
		};

		for (size_t i = 0 ; i < sizeof(testCases) / sizeof(testCases[0]) ; ++i) {

			const TestCase& tc = testCases[i];
			const vmime::string data(tc.data);

			std::ostringstream oss;
			oss << "Test case #" << i;
			const vmime::string msg = oss.str();

			vmime::parsingContext ctx;
			ctx.setHeaderParseErrorRecoveryMethod(tc.recovery);

			vmime::header hdr1;
			size_t end1 = 0;
			hdr1.parse(ctx, data, 0, data.length(), &end1);

			vmime::header hdr2;
			size_t end2 = 0;
			hdr2.parse(ctx, vmime::make_shared <vmime::utility::inputStreamStringAdapter>(data), 0, data.length(), &end2);

			VASSERT_EQ(msg + ": field count", tc.fieldCount, hdr1.getFieldCount());
			VASSERT_EQ(msg + ": end", tc.end, end1);

			VASSERT_EQ(msg + ": stream field count", hdr1.getFieldCount(), hdr2.getFieldCount());
			VASSERT_EQ(msg + ": stream end", end1, end2);
			VASSERT_EQ(msg + ": stream length", hdr1.getParsedLength(), hdr2.getParsedLength());
			VASSERT_EQ(msg + ": stream generate", hdr1.generate(), hdr2.generate());
		}
	}

VMIME_TEST_SUITE_END
