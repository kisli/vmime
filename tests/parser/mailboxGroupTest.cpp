//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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


VMIME_TEST_SUITE_BEGIN(mailboxGroupTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseExtraWhitespaces)
		VMIME_TEST(testParseNoEndDelimiter)
		VMIME_TEST(testParseExtraChars)
		VMIME_TEST(testEmptyGroup)
	VMIME_TEST_LIST_END


	void testParseExtraWhitespaces()
	{
		vmime::mailboxGroup mgrp;
		mgrp.parse(" \t group  : aaa <aaa@vmime.org>, bbb <bbb@vmime.org>");

		VASSERT_EQ("name", "group", mgrp.getName().getWholeBuffer());
		VASSERT_EQ("count", 2, mgrp.getMailboxCount());

		VASSERT_EQ("mbox1.email", "aaa@vmime.org", mgrp.getMailboxAt(0)->getEmail());
		VASSERT_EQ("mbox1.name", "aaa", mgrp.getMailboxAt(0)->getName());

		VASSERT_EQ("mbox2.email", "bbb@vmime.org", mgrp.getMailboxAt(1)->getEmail());
		VASSERT_EQ("mbox2.name", "bbb", mgrp.getMailboxAt(1)->getName());
	}

	void testParseNoEndDelimiter()
	{
		vmime::addressList addrs;
		addrs.parse("group: aaa <aaa@vmime.org>, bbb <bbb@vmime.org>");

		VASSERT_EQ("count", 1, addrs.getAddressCount());
		VASSERT_TRUE("is group", addrs.getAddressAt(0)->isGroup());

		vmime::shared_ptr <vmime::mailboxGroup> mgrp =
			vmime::dynamicCast <vmime::mailboxGroup>(addrs.getAddressAt(0));

		VASSERT_EQ("name", "group", mgrp->getName().getWholeBuffer());
		VASSERT_EQ("count", 2, mgrp->getMailboxCount());

		VASSERT_EQ("mbox1.email", "aaa@vmime.org", mgrp->getMailboxAt(0)->getEmail());
		VASSERT_EQ("mbox1.name", "aaa", mgrp->getMailboxAt(0)->getName());

		VASSERT_EQ("mbox2.email", "bbb@vmime.org", mgrp->getMailboxAt(1)->getEmail());
		VASSERT_EQ("mbox2.name", "bbb", mgrp->getMailboxAt(1)->getName());
	}

	void testParseExtraChars()
	{
		vmime::mailboxGroup mgrp;
		mgrp.parse("group: aaa <aaa@vmime.org>, bbb <bbb@vmime.org>; extra chars here...");

		VASSERT_EQ("name", "group", mgrp.getName().getWholeBuffer());
		VASSERT_EQ("count", 2, mgrp.getMailboxCount());

		VASSERT_EQ("mbox1.email", "aaa@vmime.org", mgrp.getMailboxAt(0)->getEmail());
		VASSERT_EQ("mbox1.name", "aaa", mgrp.getMailboxAt(0)->getName());

		VASSERT_EQ("mbox2.email", "bbb@vmime.org", mgrp.getMailboxAt(1)->getEmail());
		VASSERT_EQ("mbox2.name", "bbb", mgrp.getMailboxAt(1)->getName());
	}

	void testEmptyGroup()
	{
		vmime::mailboxGroup mgrp;
		mgrp.parse("Undisclosed recipients:;");

		VASSERT_EQ("name", "Undisclosed recipients", mgrp.getName().getWholeBuffer());
		VASSERT_EQ("count", 0, mgrp.getMailboxCount());
	}

VMIME_TEST_SUITE_END
