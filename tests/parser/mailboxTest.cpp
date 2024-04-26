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


VMIME_TEST_SUITE_BEGIN(mailboxTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testEmptyEmailAddress)
		VMIME_TEST(testSeparatorInComment)
		VMIME_TEST(testMalformations)
		VMIME_TEST(testExcessiveQuoting)
		VMIME_TEST(testSpacing)
	VMIME_TEST_LIST_END


	void testParse() {

		static const vmime::string testSuitesParse[] = {

			// Test 1
			"My (this is a comment)name <me(another \\)comment) @    	somewhere(else).com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=My name]]], email=me@somewhere.com]]]",

			// Test 2
			"mailbox1 <mailbox@one>,;,,,	,,  	,,;group1:mailbox1@group1,  mailbox2@group2,,\"mailbox #3\" <mailbox3@group2>;, <mailbox@two>,,,,,,,,=?iso-8859-1?q?mailbox_number_3?= <mailbox@three>, =?abc?Q?mailbox?=   	 =?def?Q?_number_4?= <mailbox@four>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=mailbox1]]], email=mailbox@one],[mailbox-group: name=[text: [[word: charset=us-ascii, buffer=group1]]], list=[[mailbox: name=[text: []], email=mailbox1@group1],[mailbox: name=[text: []], email=mailbox2@group2],[mailbox: name=[text: [[word: charset=us-ascii, buffer=mailbox #3]]], email=mailbox3@group2]]],[mailbox: name=[text: []], email=mailbox@two],[mailbox: name=[text: [[word: charset=iso-8859-1, buffer=mailbox number 3]]], email=mailbox@three],[mailbox: name=[text: [[word: charset=abc, buffer=mailbox],[word: charset=def, buffer= number 4]]], email=mailbox@four]]]",

			// Test 3
			"John Doe <john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=John Doe]]], email=john.doe@acme.com]]]",

			// Test 4
			"john.doe@acme.com (John Doe)",

			"[address-list: [[mailbox: name=[text: []], email=john.doe@acme.com]]]",

			// Test 5
			"John.Doe(ignore)@acme.com (John Doe)",

			"[address-list: [[mailbox: name=[text: []], email=John.Doe@acme.com]]]",

			// Test 6
			"<john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: []], email=john.doe@acme.com]]]",

			// Test 7
			"john.doe@acme.com",

			"[address-list: [[mailbox: name=[text: []], email=john.doe@acme.com]]]",

			// Test 8
			"\"John Doe\" <john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=John Doe]]], email=john.doe@acme.com]]]",

			// Test 9
			"=?us-ascii?q?John?=<john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=John]]], email=john.doe@acme.com]]]",

			// Test 10
			"\"John\"<john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=John]]], email=john.doe@acme.com]]]",

			// Test 11
			"John<john.doe@acme.com>",

			"[address-list: [[mailbox: name=[text: [[word: charset=us-ascii, buffer=John]]], email=john.doe@acme.com]]]"
		};

		for (unsigned int i = 0 ; i < sizeof(testSuitesParse) / sizeof(testSuitesParse[0]) / 2 ; ++i) {

			vmime::string in = testSuitesParse[i * 2];
			vmime::string out = testSuitesParse[i * 2 + 1];

			std::ostringstream oss;
			oss << "Test " << (i + 1);

			vmime::addressList addrList;
			addrList.parse(in);

			std::ostringstream cmp;
			cmp << addrList;

			VASSERT_EQ(oss.str(), out, cmp.str());
		}
	}

	void testEmptyEmailAddress() {

		vmime::addressList addrList;
		addrList.parse("\"Full Name\" <>");

		VASSERT_EQ("count", 1, addrList.getAddressCount());
		VASSERT_EQ("!group", false, addrList.getAddressAt(0)->isGroup());

		vmime::shared_ptr <vmime::mailbox> mbox =
			vmime::dynamicCast <vmime::mailbox>(addrList.getAddressAt(0));

		VASSERT_EQ("name", "Full Name", mbox->getName());
		VASSERT_EQ("email", "", mbox->getEmail());
	}

	void testSeparatorInComment() {

		vmime::addressList addrList;
		addrList.parse("aaa(comment,comment)@vmime.org, bbb@vmime.org");

		VASSERT_EQ("count", 2, addrList.getAddressCount());

		vmime::shared_ptr <vmime::mailbox> mbox1 =
			vmime::dynamicCast <vmime::mailbox>(addrList.getAddressAt(0));
		vmime::shared_ptr <vmime::mailbox> mbox2 =
			vmime::dynamicCast <vmime::mailbox>(addrList.getAddressAt(1));

		VASSERT_EQ("name1", vmime::text(), mbox1->getName());
		VASSERT_EQ("email1", "aaa@vmime.org", mbox1->getEmail());

		VASSERT_EQ("name2", vmime::text(), mbox2->getName());
		VASSERT_EQ("email2", "bbb@vmime.org", mbox2->getEmail());
	}

	void testMalformations() {
		vmime::mailbox mbox;

		mbox.parse("a@b.c <e@f.g>");
		VASSERT_EQ("name", vmime::text("a@b.c"), mbox.getName());
		VASSERT_EQ("email", "e@f.g", mbox.getEmail());

		mbox.parse("a@b.c e@f.g <h@i.j>");
		VASSERT_EQ("name", vmime::text("a@b.c e@f.g"), mbox.getName());
		VASSERT_EQ("email", "h@i.j", mbox.getEmail());

		mbox.parse("Foo <bar<baz@quux.com>");
		VASSERT_EQ("name", vmime::text("Foo <bar"), mbox.getName());
		VASSERT_EQ("email", "baz@quux.com", mbox.getEmail());

		mbox.parse("Foo <foo@x.com> <bar@x.com>");
		VASSERT_EQ("name", vmime::text("Foo <foo@x.com>"), mbox.getName());
		VASSERT_EQ("email", "bar@x.com", mbox.getEmail());

		mbox.parse("Foo <foo@x.com> Bar <bar@y.com>");
		VASSERT_EQ("name", vmime::text("Foo <foo@x.com> Bar"), mbox.getName());
		VASSERT_EQ("email", "bar@y.com", mbox.getEmail());
	}

	void testExcessiveQuoting() {
		using namespace vmime;

		// Check that ASCII display names are not encoded more than necessary
		emailAddress e("a@b.com");
		auto a = make_shared<mailbox>(text(word("Foo B@r", charsets::US_ASCII)), e);
		VASSERT_EQ("generate", "\"Foo B@r\" <a@b.com>", a->generate());
		VASSERT_NEQ("generate", "=?utf-8?Q?Foo_B=40r?= <a@b.com>", a->generate());

		a = make_shared<mailbox>(text(word("Foo B@r", charsets::UTF_8)), e);
		VASSERT_EQ("generate", "=?utf-8?Q?Foo_B=40r?= <a@b.com>", a->generate());
	}

	void testSpacing() {

		vmime::text t("Foo B\xc3\xa4renstark Baz", vmime::charsets::UTF_8);
		vmime::mailbox m(t, "a@b.de");
		VASSERT_EQ("1", "Foo =?utf-8?Q?B=C3=A4renstark?= Baz", t.generate());
		VASSERT_EQ("2", "=?us-ascii?Q?Foo?= =?utf-8?Q?_B=C3=A4renstark?= =?us-ascii?Q?_Baz?= <a@b.de>", m.generate());

	}

VMIME_TEST_SUITE_END
