//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/handler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	class mailboxTest : public suite
	{
		void testParse()
		{
			static const vmime::string testSuitesParse[] =
			{
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
				"John.Doe (ignore) @acme.com (John Doe)",

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
			};

			for (unsigned int i = 0 ; i < sizeof(testSuitesParse) / sizeof(testSuitesParse[0]) / 2 ; ++i)
			{
				vmime::string in = testSuitesParse[i * 2];
				vmime::string out = testSuitesParse[i * 2 + 1];

				std::ostringstream oss;
				oss << "Test " << (i + 1);

				vmime::addressList addrList;
				addrList.parse(in);

				std::ostringstream cmp;
				cmp << addrList;

				assert_eq(oss.str(), out, cmp.str());
			}
		}

	public:

		mailboxTest() : suite("vmime::mailbox")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &mailboxTest::testParse));

			suite::main().add("vmime::mailbox", this);
		}

	};

	mailboxTest* theTest = new mailboxTest();
}
