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


#define VMIME_TEST_SUITE         headerTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testHas1)
		VMIME_TEST(testHas2)

		VMIME_TEST(testAppend1)
		VMIME_TEST(testAppend2)

		VMIME_TEST(testInsertFieldBefore1)
		VMIME_TEST(testInsertFieldBefore2)

		VMIME_TEST(testInsertFieldAfter1)
		VMIME_TEST(testInsertFieldAfter2)

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
	VMIME_TEST_LIST_END


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

		VASSERT_EQ("Value", false, res);
	}

	void testHas2()
	{
		vmime::header hdr;
		hdr.parse("X: x\r\nTo: y\r\nTo: z\r\n");

		bool res = hdr.hasField("To");

		VASSERT_EQ("Value", true, res);
	}

	// appendField function tests
	void testAppend1()
	{
		vmime::header hdr;
		hdr.parse("");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("A", "a");
		hdr.appendField(hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(1), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
	}

	void testAppend2()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\n");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.appendField(hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
	}

	// insertFieldBefore
	void testInsertFieldBefore1()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldBefore(hdr.getField("C"), hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	void testInsertFieldBefore2()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldBefore(1, hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	// insertFieldAfter
	void testInsertFieldAfter1()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldAfter(hdr.getField("A"), hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	void testInsertFieldAfter2()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nC: c\r\n");

		vmime::ref <vmime::headerField> hf = vmime::headerFieldFactory::getInstance()->create("B", "b");
		hdr.insertFieldAfter(0, hf);

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
	}

	// removeField
	void testRemoveField1()
	{
		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\nB: b\r\nC: c\r\n");
		hdr2.parse("A: a\r\nB: b\r\nC: c\r\n");

		hdr1.removeField(hdr1.getField("B"));
		hdr2.removeField(1);

		std::vector <vmime::ref <vmime::headerField> > res1 = hdr1.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res1.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res1[0]));
		VASSERT_EQ("Second value", "C: c", headerTest::getFieldValue(*res1[1]));

		std::vector <vmime::ref <vmime::headerField> > res2 = hdr2.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res2.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res2[0]));
		VASSERT_EQ("Second value", "C: c", headerTest::getFieldValue(*res2[1]));
	}

	void testRemoveField2()
	{
		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\n");
		hdr2.parse("A: a\r\n");

		hdr1.removeField(hdr1.getField("A"));
		hdr2.removeField(0);

		std::vector <vmime::ref <vmime::headerField> > res1 = hdr1.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res1.size());

		std::vector <vmime::ref <vmime::headerField> > res2 = hdr2.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res2.size());
	}

	// removeAllFields
	void testRemoveAllFields()
	{
		vmime::header hdr1, hdr2;
		hdr1.parse("A: a\r\n");
		hdr2.parse("A: a\r\nB: b\r\n");

		hdr1.removeAllFields();
		hdr2.removeAllFields();

		std::vector <vmime::ref <vmime::headerField> > res1 = hdr1.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res1.size());

		std::vector <vmime::ref <vmime::headerField> > res2 = hdr2.getFieldList();
		VASSERT_EQ("Count", static_cast <unsigned int>(0), res2.size());
	}

	// getFieldCount
	void testgetFieldCount()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\nD: d\r\n");

		VASSERT_EQ("Value", 4, hdr.getFieldCount());
	}

	// isEmpty
	void testIsEmpty1()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\n");

		VASSERT_EQ("Value", false, hdr.isEmpty());
	}

	void testIsEmpty2()
	{
		vmime::header hdr;
		hdr.parse("\r\n");

		VASSERT_EQ("Value", true, hdr.isEmpty());
	}

	// getFieldAt
	void testGetFieldAt()
	{
		vmime::header hdr;
		hdr.parse("B: b\r\nA: a\r\nC: c\r\n");

		vmime::ref <vmime::headerField> res = hdr.getFieldAt(2);

		VASSERT_EQ("Value", "C: c", getFieldValue(*res));
	}

	// getFieldList
	void testGetFieldList1()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nB: b1\r\nC: c\r\nB: b2\r\n");

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(4), res.size());
		VASSERT_EQ("First value", "A: a", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b1", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Third value", "C: c", headerTest::getFieldValue(*res[2]));
		VASSERT_EQ("Thourth value", "B: b2", headerTest::getFieldValue(*res[3]));
	}

	void testGetFieldList2()
	{
		vmime::header hdr;
		hdr.parse("\r\n");

		std::vector <vmime::ref <vmime::headerField> > res = hdr.getFieldList();

		VASSERT_EQ("Count", static_cast <unsigned int>(0), res.size());
	}

	// find function tests
	void testFind1()
	{
		vmime::header hdr;
		hdr.parse("A: a\r\nB: b\r\nC: c\r\nB: d\r\n");

		vmime::ref <vmime::headerField> res = hdr.findField("B");

		VASSERT_EQ("Value", "B: b", getFieldValue(*res));
	}

	// getAllByName function tests
	void testFindAllFields1()
	{
		vmime::header hdr;
		hdr.parse("A: a1\nC: c1\n");

		std::vector <vmime::ref <vmime::headerField> > res = hdr.findAllFields("B");

		VASSERT_EQ("Count", static_cast <unsigned int>(0), res.size());
	}

	void testFindAllFields2()
	{
		vmime::header hdr;
		hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\n");

		std::vector <vmime::ref <vmime::headerField> > res = hdr.findAllFields("B");

		VASSERT_EQ("Count", static_cast <unsigned int>(2), res.size());
		VASSERT_EQ("First value", "B: b1", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "B: b2", headerTest::getFieldValue(*res[1]));
	}

	void testFindAllFields3()
	{
		vmime::header hdr;
		hdr.parse("A: a1\nB: b1\nB: b2\nC: c1\nC: c3\nC: c2\n");

		std::vector <vmime::ref <vmime::headerField> > res = hdr.findAllFields("C");

		VASSERT_EQ("Count", static_cast <unsigned int>(3), res.size());
		VASSERT_EQ("First value", "C: c1", headerTest::getFieldValue(*res[0]));
		VASSERT_EQ("Second value", "C: c3", headerTest::getFieldValue(*res[1]));
		VASSERT_EQ("Second value", "C: c2", headerTest::getFieldValue(*res[2]));
	}

VMIME_TEST_SUITE_END

