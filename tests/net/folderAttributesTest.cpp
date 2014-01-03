//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2014 Vincent Richard <vincent@vmime.org>
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

#include "vmime/net/folderAttributes.hpp"


VMIME_TEST_SUITE_BEGIN(folderAttributesTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstruct)
		VMIME_TEST(testConstructCopy)
		VMIME_TEST(testSetType)
		VMIME_TEST(testSetFlags)
		VMIME_TEST(testHasFlag)
		VMIME_TEST(testSetUserFlags)
		VMIME_TEST(testHasUserFlag)
		VMIME_TEST(testSetSpecialUse)
	VMIME_TEST_LIST_END


	void testConstruct()
	{
		vmime::net::folderAttributes attr;

		// Default values
		VASSERT_EQ("type", vmime::net::folderAttributes::TYPE_CONTAINS_FOLDERS
			| vmime::net::folderAttributes::TYPE_CONTAINS_MESSAGES, attr.getType());
		VASSERT_EQ("flags", 0, attr.getFlags());
		VASSERT_EQ("user-flags", 0, attr.getUserFlags().size());
		VASSERT_EQ("special-use", vmime::net::folderAttributes::SPECIALUSE_NONE, attr.getSpecialUse());
	}

	void testConstructCopy()
	{
		std::vector <vmime::string> userFlags;
		userFlags.push_back("\\XMyFlag1");
		userFlags.push_back("\\XMyFlag2");
		userFlags.push_back("\\XMyFlag3");

		vmime::net::folderAttributes attr;

		attr.setFlags(vmime::net::folderAttributes::FLAG_HAS_CHILDREN);
		attr.setUserFlags(userFlags);

		vmime::net::folderAttributes attr2(attr);

		VASSERT("flags", attr2.getFlags() == attr.getFlags());
		VASSERT("user-flags", attr2.getUserFlags() == attr.getUserFlags());
	}

	void testSetType()
	{
		vmime::net::folderAttributes attr;
		attr.setType(vmime::net::folderAttributes::TYPE_CONTAINS_FOLDERS);

		VASSERT_EQ("eq", vmime::net::folderAttributes::TYPE_CONTAINS_FOLDERS, attr.getType());
	}

	void testSetFlags()
	{
		vmime::net::folderAttributes attr;
		attr.setFlags(vmime::net::folderAttributes::FLAG_HAS_CHILDREN);

		VASSERT_EQ("eq", vmime::net::folderAttributes::FLAG_HAS_CHILDREN, attr.getFlags());
	}

	void testHasFlag()
	{
		vmime::net::folderAttributes attr;
		attr.setFlags(vmime::net::folderAttributes::FLAG_HAS_CHILDREN);

		VASSERT("has", attr.hasFlag(vmime::net::folderAttributes::FLAG_HAS_CHILDREN));
		VASSERT("has-not", !attr.hasFlag(vmime::net::folderAttributes::FLAG_NO_OPEN));
	}

	void testSetUserFlags()
	{
		std::vector <vmime::string> userFlags;
		userFlags.push_back("\\XMyFlag1");
		userFlags.push_back("\\XMyFlag2");
		userFlags.push_back("\\XMyFlag3");

		vmime::net::folderAttributes attr;
		attr.setUserFlags(userFlags);

		VASSERT("eq", attr.getUserFlags() == userFlags);
	}

	void testHasUserFlag()
	{
		std::vector <vmime::string> userFlags;
		userFlags.push_back("\\XMyFlag1");
		userFlags.push_back("\\XMyFlag2");
		userFlags.push_back("\\XMyFlag3");

		vmime::net::folderAttributes attr;
		attr.setUserFlags(userFlags);

		VASSERT("has", attr.hasUserFlag("\\XMyFlag1"));
		VASSERT("has-casesensitive", !attr.hasUserFlag("\\xmyflag1"));
		VASSERT("has-not", !attr.hasUserFlag("\\XMyFlag4"));
	}

	void testSetSpecialUse()
	{
		const int use = vmime::net::folderAttributes::SPECIALUSE_JUNK
			| vmime::net::folderAttributes::SPECIALUSE_TRASH;

		vmime::net::folderAttributes attr;
		attr.setSpecialUse(use);

		VASSERT_EQ("eq", use, attr.getSpecialUse());
	}

VMIME_TEST_SUITE_END
