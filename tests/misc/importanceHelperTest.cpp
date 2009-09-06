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

#include "vmime/misc/importanceHelper.hpp"


#define VMIME_TEST_SUITE         importanceHelperTest
#define VMIME_TEST_SUITE_MODULE  "Misc"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testResetImportance)

		VMIME_TEST(testSetImportance1)
		VMIME_TEST(testSetImportance2)
		VMIME_TEST(testSetImportance3)
		VMIME_TEST(testSetImportance4)
		VMIME_TEST(testSetImportance5)

		VMIME_TEST(testGetImportance1)
		VMIME_TEST(testGetImportance2)
		VMIME_TEST(testGetImportance3)
		VMIME_TEST(testGetImportance4)
		VMIME_TEST(testGetImportance5)
	VMIME_TEST_LIST_END


	// resetImportance

	void testResetImportance()
	{
		vmime::ref <vmime::header> hdr = vmime::create <vmime::header>();

		hdr->getField("Importance")->setValue("xxx");
		hdr->getField("X-Priority")->setValue("yyy");

		VASSERT_NO_THROW("1", hdr->findField("Importance"));
		VASSERT_NO_THROW("2", hdr->findField("X-Priority"));

		vmime::misc::importanceHelper::resetImportanceHeader(hdr);

		VASSERT_THROW("3", hdr->findField("Importance"), vmime::exceptions::no_such_field);
		VASSERT_THROW("4", hdr->findField("X-Priority"), vmime::exceptions::no_such_field);
	}


	// setImportance

	void testSetImportanceImpl(const vmime::misc::importanceHelper::Importance i,
		const std::string& ImportanceValue, const std::string& XPriorityValue)
	{
		vmime::ref <vmime::header> hdr = vmime::create <vmime::header>();

		vmime::misc::importanceHelper::setImportanceHeader(hdr, i);

		VASSERT_NO_THROW("1", hdr->findField("Importance"));
		VASSERT_EQ("2", ImportanceValue, hdr->findField("Importance")->getValue()->generate());

		VASSERT_NO_THROW("3", hdr->findField("X-Priority"));
		VASSERT_EQ("4", XPriorityValue, hdr->findField("X-Priority")->getValue()->generate());
	}

	void testSetImportance1()
	{
		testSetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_HIGHEST,
			"high", "1 (Highest)");
	}

	void testSetImportance2()
	{
		testSetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_HIGH,
			"high", "2 (High)");
	}

	void testSetImportance3()
	{
		testSetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_NORMAL,
			"normal", "3 (Normal)");
	}

	void testSetImportance4()
	{
		testSetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_LOW,
			"low", "4 (Low)");
	}

	void testSetImportance5()
	{
		testSetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_LOWEST,
			"low", "5 (Lowest)");
	}


	// getImportance

	void testGetImportanceImpl(const vmime::misc::importanceHelper::Importance i1,
		const vmime::misc::importanceHelper::Importance i2,
		const std::string& ImportanceValue, const std::string& XPriorityValue)
	{
		vmime::ref <vmime::header> hdr1 = vmime::create <vmime::header>();

		hdr1->getField("Importance")->setValue(ImportanceValue);
		VASSERT_EQ("1", i1, vmime::misc::importanceHelper::getImportanceHeader(hdr1));

		vmime::ref <vmime::header> hdr2 = vmime::create <vmime::header>();

		hdr2->getField("X-Priority")->setValue(XPriorityValue);
		VASSERT_EQ("2", i2, vmime::misc::importanceHelper::getImportanceHeader(hdr2));
	}

	void testGetImportance1()
	{
		testGetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_HIGHEST,
			vmime::misc::importanceHelper::IMPORTANCE_HIGHEST, "high", "1 (Highest)");
	}

	void testGetImportance2()
	{
		testGetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_HIGHEST,
			vmime::misc::importanceHelper::IMPORTANCE_HIGH, "high", "2 (High)");
	}

	void testGetImportance3()
	{
		testGetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_NORMAL,
			vmime::misc::importanceHelper::IMPORTANCE_NORMAL, "normal", "3 (Normal)");
	}

	void testGetImportance4()
	{
		testGetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_LOWEST,
			vmime::misc::importanceHelper::IMPORTANCE_LOW, "low", "4 (Low)");
	}

	void testGetImportance5()
	{
		testGetImportanceImpl(vmime::misc::importanceHelper::IMPORTANCE_LOWEST,
			vmime::misc::importanceHelper::IMPORTANCE_LOWEST, "low", "5 (Lowest)");
	}

VMIME_TEST_SUITE_END

