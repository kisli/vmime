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


VMIME_TEST_SUITE_BEGIN(mailboxListTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseGroup)
	VMIME_TEST_LIST_END


	// Disposition-Notification-To:: <email@domain.com>
	void testParseGroup()
	{
		// Groups should be extracted to multiple mailboxes in mailbox lists
		vmime::mailboxList mboxList;
		mboxList.parse("email1@domain1.com, : <email2@domain2.com>, email3@domain3.com");

		VASSERT_EQ("count", 3, mboxList.getMailboxCount());
		VASSERT_EQ("email", "email1@domain1.com", mboxList.getMailboxAt(0)->getEmail().generate());
		VASSERT_EQ("email", "email2@domain2.com", mboxList.getMailboxAt(1)->getEmail().generate());
		VASSERT_EQ("email", "email3@domain3.com", mboxList.getMailboxAt(2)->getEmail().generate());
	}

VMIME_TEST_SUITE_END

