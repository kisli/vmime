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


VMIME_TEST_SUITE_BEGIN(mailboxListTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseGroup)
		VMIME_TEST(testBrokenGroup)
		VMIME_TEST(testGenerateFolding)
		VMIME_TEST(testParseQuotedSpecials)
	VMIME_TEST_LIST_END


	// Disposition-Notification-To:: <email@domain.com>
	void testParseGroup() {

		// Groups should be extracted to multiple mailboxes in mailbox lists
		vmime::mailboxList mboxList;
		mboxList.parse("email1@domain1.com, : <email2@domain2.com>, email3@domain3.com");

		VASSERT_EQ("count", 3, mboxList.getMailboxCount());
		VASSERT_EQ("email", "email1@domain1.com", mboxList.getMailboxAt(0)->getEmail().generate());
		VASSERT_EQ("email", "email2@domain2.com", mboxList.getMailboxAt(1)->getEmail().generate());
		VASSERT_EQ("email", "email3@domain3.com", mboxList.getMailboxAt(2)->getEmail().generate());
	}

	void testBrokenGroup() {

		std::string bad(":,");

		for (int i = 0 ; i < 10 ; ++i) {
			bad = bad + bad;
		}

		vmime::mailboxList mboxList;
		mboxList.parse(bad);

		VASSERT_EQ("count", 0, mboxList.getMailboxCount());
	}

	void testGenerateFolding() {

		using namespace vmime;

		mailboxList ml;
		ml.appendMailbox(make_shared <mailbox>(text("Alice Allison", charsets::UTF_8), emailAddress("alice@example.com")));
		ml.appendMailbox(make_shared <mailbox>(text("Bob Builder", charsets::UTF_8), emailAddress("bob@example.com")));
		ml.appendMailbox(make_shared <mailbox>(text("Doe, John", charsets::UTF_8), emailAddress("john@example.com")));
		ml.appendMailbox(make_shared <mailbox>(text("O'Brien; Pat", charsets::UTF_8), emailAddress("pat@example.com")));
		ml.appendMailbox(make_shared <mailbox>(text("Carol <c> Smith", charsets::UTF_8), emailAddress("carol@example.com")));
		ml.appendMailbox(make_shared <mailbox>(text("J\xc3\xbcrgen M\xc3\xbcller", charsets::UTF_8), emailAddress("juergen@example.com")));
		ml.appendMailbox(make_shared <mailbox>(emailAddress("first.person@example.com")));

		// Whole mailboxes move to the next line, the fold replaces the
		// space after the comma
		VASSERT_EQ(
			"1",
			"\"Alice Allison\" <alice@example.com>, \"Bob Builder\" <bob@example.com>,\r\n"
			" \"Doe, John\" <john@example.com>, \"O'Brien; Pat\" <pat@example.com>,\r\n"
			" \"Carol <c> Smith\" <carol@example.com>,\r\n"
			" =?utf-8?Q?J=C3=BCrgen_M=C3=BCller?= <juergen@example.com>,\r\n"
			" first.person@example.com",
			ml.generate(78, 4)
		);

		mailboxList back;
		back.parse(ml.generate(78, 4));
		VASSERT_EQ("2", 7, back.getMailboxCount());
		VASSERT_EQ("3", "Doe, John", back.getMailboxAt(2)->getName().getWholeBuffer());

		VASSERT_EQ(
			"4",
			"\"Alice Allison\" <alice@example.com>, \"Bob Builder\" <bob@example.com>, "
			"\"Doe, John\" <john@example.com>, \"O'Brien; Pat\" <pat@example.com>, "
			"\"Carol <c> Smith\" <carol@example.com>, "
			"=?utf-8?Q?J=C3=BCrgen_M=C3=BCller?= <juergen@example.com>, "
			"first.person@example.com",
			ml.generate()
		);
	}

	void testParseQuotedSpecials() {

		// '(' inside a quoted-string does not start a comment
		vmime::mailboxList ml1;
		ml1.parse("\"F\" <f@x.com>, \"a(b\" <r@x.com>, \"Z\" <s@x.com>");

		VASSERT_EQ("1", 3, ml1.getMailboxCount());
		VASSERT_EQ("2", "a(b", ml1.getMailboxAt(1)->getName().getWholeBuffer());
		VASSERT_EQ("3", "r@x.com", ml1.getMailboxAt(1)->getEmail().generate());

		// '"' inside a comment does not start a quoted-string
		vmime::mailboxList ml2;
		ml2.parse("(a\"b) <r@x.com>, <s@x.com>");

		VASSERT_EQ("4", 2, ml2.getMailboxCount());
		VASSERT_EQ("5", "s@x.com", ml2.getMailboxAt(1)->getEmail().generate());
	}

VMIME_TEST_SUITE_END
