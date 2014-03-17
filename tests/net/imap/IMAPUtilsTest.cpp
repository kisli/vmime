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

#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPParser.hpp"


using namespace vmime::net::imap;



VMIME_TEST_SUITE_BEGIN(IMAPUtilsTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testQuoteString)
		VMIME_TEST(testToModifiedUTF7)
		VMIME_TEST(testFromModifiedUTF7)
		VMIME_TEST(testConvertAddressList)
		VMIME_TEST(testMessageFlagList)
		VMIME_TEST(testDateTime)
		VMIME_TEST(testPathToString)
		VMIME_TEST(testStringToPath)
		VMIME_TEST(testBuildFetchCommand)
	VMIME_TEST_LIST_END


	void testQuoteString()
	{
		VASSERT_EQ("unquoted", "ascii", IMAPUtils::quoteString("ascii"));
		VASSERT_EQ("space", "\"ascii with space\"", IMAPUtils::quoteString("ascii with space"));

		VASSERT_EQ("special1", "\"(\"", IMAPUtils::quoteString("("));
		VASSERT_EQ("special2", "\")\"", IMAPUtils::quoteString(")"));
		VASSERT_EQ("special3", "\"{\"", IMAPUtils::quoteString("{"));
		VASSERT_EQ("special4", "\" \"", IMAPUtils::quoteString(" "));
		VASSERT_EQ("special5", "\"%\"", IMAPUtils::quoteString("%"));
		VASSERT_EQ("special6", "\"*\"", IMAPUtils::quoteString("*"));
		VASSERT_EQ("special7", "\"\\\"\"", IMAPUtils::quoteString("\""));
		VASSERT_EQ("special8", "\"\\\\\"", IMAPUtils::quoteString("\\"));
		VASSERT_EQ("special9", "\"\x7f\"", IMAPUtils::quoteString("\x7f"));

	}

	void testToModifiedUTF7()
	{
		#define FC(x) vmime::net::folder::path::component(x, vmime::charsets::UTF_8)

		// Example strings from RFC-1642 (modified for IMAP UTF-7)
		VASSERT_EQ("1", "A&ImIDkQ-.", IMAPUtils::toModifiedUTF7('/', FC("A\xe2\x89\xa2\xce\x91.")));
		VASSERT_EQ("2", "Hi Mum &Jjo-!", IMAPUtils::toModifiedUTF7('/', FC("Hi Mum \xe2\x98\xba!")));
		VASSERT_EQ("3", "&ZeVnLIqe-", IMAPUtils::toModifiedUTF7('/', FC("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e")));
		VASSERT_EQ("4", "Item 3 is &AKM- 1.", IMAPUtils::toModifiedUTF7('/', FC("Item 3 is \xc2\xa3 1.")));

		VASSERT_EQ("escape char", "&-", IMAPUtils::toModifiedUTF7('/', FC("&")));
		VASSERT_EQ("ascii", "plain ascii text", IMAPUtils::toModifiedUTF7('/', FC("plain ascii text")));
		VASSERT_EQ("special", "!\"#$%*+-;<=>@[]^_`{|}", IMAPUtils::toModifiedUTF7('/', FC("!\"#$%*+-;<=>@[]^_`{|}")));

		#undef FC
	}

	void testFromModifiedUTF7()
	{
		#define FC(x) vmime::net::folder::path::component(x, vmime::charsets::UTF_8)

		// Example strings from RFC-1642 (modified for IMAP UTF-7)
		VASSERT_EQ("1", FC("A\xe2\x89\xa2\xce\x91."), IMAPUtils::fromModifiedUTF7("A&ImIDkQ-."));
		VASSERT_EQ("2", FC("Hi Mum \xe2\x98\xba!"), IMAPUtils::fromModifiedUTF7("Hi Mum &Jjo-!"));
		VASSERT_EQ("3", FC("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e"), IMAPUtils::fromModifiedUTF7("&ZeVnLIqe-"));
		VASSERT_EQ("4", FC("Item 3 is \xc2\xa3 1."), IMAPUtils::fromModifiedUTF7("Item 3 is &AKM- 1."));

		VASSERT_EQ("escape char", FC("&"), IMAPUtils::fromModifiedUTF7("&-"));
		VASSERT_EQ("ascii", FC("plain ascii text"), IMAPUtils::fromModifiedUTF7("plain ascii text"));
		VASSERT_EQ("special", FC("!\"#$%*+;<=>@[]^_`{|}"), IMAPUtils::fromModifiedUTF7("!\"#$%*+-;<=>@[]^_`{|}"));

		#undef FC
	}

	void testConvertAddressList()
	{
		IMAPParser parser;
		IMAPParser::address_list addrList;

		vmime::string line("((\"name\" NIL \"mailbox\" \"host\")(\"name2\" NIL \"mailbox2\" \"host2\"))");
		vmime::size_t pos = 0;

		VASSERT("parse", addrList.parseImpl(parser, line, &pos));

		vmime::mailboxList mboxList;
		IMAPUtils::convertAddressList(addrList, mboxList);

		VASSERT_EQ("mbox-count", 2, mboxList.getMailboxCount());
		VASSERT_EQ("mbox-1", "mailbox@host", mboxList.getMailboxAt(0)->getEmail().toString());
		VASSERT_EQ("mbox-1", "name", mboxList.getMailboxAt(0)->getName().getWholeBuffer());
		VASSERT_EQ("mbox-2", "mailbox2@host2", mboxList.getMailboxAt(1)->getEmail().toString());
		VASSERT_EQ("mbox-2", "name2", mboxList.getMailboxAt(1)->getName().getWholeBuffer());
	}

	void testMessageFlagList()
	{
		int flags = 0;
		std::vector <vmime::string> flagList;

		// Test each flag
		flags = vmime::net::message::FLAG_REPLIED;
		flagList = IMAPUtils::messageFlagList(flags);
		VASSERT("replied", std::find(flagList.begin(), flagList.end(), "\\Answered") != flagList.end());

		flags = vmime::net::message::FLAG_MARKED;
		flagList = IMAPUtils::messageFlagList(flags);
		VASSERT("marked", std::find(flagList.begin(), flagList.end(), "\\Flagged") != flagList.end());

		flags = vmime::net::message::FLAG_DELETED;
		flagList = IMAPUtils::messageFlagList(flags);
		VASSERT("deleted", std::find(flagList.begin(), flagList.end(), "\\Deleted") != flagList.end());

		flags = vmime::net::message::FLAG_SEEN;
		flagList = IMAPUtils::messageFlagList(flags);
		VASSERT("seen", std::find(flagList.begin(), flagList.end(), "\\Seen") != flagList.end());

		flags = vmime::net::message::FLAG_DRAFT;
		flagList = IMAPUtils::messageFlagList(flags);
		VASSERT("draft", std::find(flagList.begin(), flagList.end(), "\\Draft") != flagList.end());

		// Multiple flags
		flags = vmime::net::message::FLAG_REPLIED;
		flagList = IMAPUtils::messageFlagList(flags);

		VASSERT_EQ("1.size", 1, flagList.size());
		VASSERT("1.found", std::find(flagList.begin(), flagList.end(), "\\Answered") != flagList.end());

		flags |= vmime::net::message::FLAG_SEEN;
		flagList = IMAPUtils::messageFlagList(flags);

		VASSERT_EQ("2.size", 2, flagList.size());
		VASSERT("2.found1", std::find(flagList.begin(), flagList.end(), "\\Answered") != flagList.end());
		VASSERT("2.found2", std::find(flagList.begin(), flagList.end(), "\\Seen") != flagList.end());
	}

	void testDateTime()
	{
		vmime::datetime dt(2014, 3, 17, 23, 26, 22, vmime::datetime::GMT2);
		VASSERT_EQ("datetime", "\"17-Mar-2014 23:26:22 +0200\"", IMAPUtils::dateTime(dt));
	}

	void testPathToString()
	{
		#define FC(x) vmime::net::folder::path::component(x, vmime::charsets::UTF_8)

		vmime::net::folder::path path;
		path /= FC("Hi Mum \xe2\x98\xba!");
		path /= FC("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");

		VASSERT_EQ("string", "Hi Mum &Jjo-!/&ZeVnLIqe-", IMAPUtils::pathToString('/', path));

		#undef FC
	}

	void testStringToPath()
	{
		#define FC(x) vmime::net::folder::path::component(x, vmime::charsets::UTF_8)

		vmime::net::folder::path path = IMAPUtils::stringToPath('/', "Hi Mum &Jjo-!/&ZeVnLIqe-");

		VASSERT_EQ("count", 2, path.getSize());
		VASSERT_EQ("component1", FC("Hi Mum \xe2\x98\xba!"), path[0]);
		VASSERT_EQ("component2", FC("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e"), path[1]);

		#undef FC
	}

	void testBuildFetchCommand()
	{
		vmime::shared_ptr <IMAPConnection> cnt;
		vmime::net::messageSet msgs = vmime::net::messageSet::byNumber(42);

		// SIZE
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::SIZE;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("size", "FETCH 42 RFC822.SIZE", cmd->getText());
		}

		// FLAGS
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::FLAGS;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("flags", "FETCH 42 FLAGS", cmd->getText());
		}

		// STRUCTURE
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::STRUCTURE;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("structure", "FETCH 42 BODYSTRUCTURE", cmd->getText());
		}

		// UID
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::UID;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("uid", "FETCH 42 UID", cmd->getText());
		}

		// ENVELOPE
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::ENVELOPE;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("envelope", "FETCH 42 ENVELOPE", cmd->getText());
		}

		// CONTENT_INFO
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::CONTENT_INFO;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("content", "FETCH 42 BODY[HEADER.FIELDS (CONTENT_TYPE)]", cmd->getText());
		}

		// IMPORTANCE
		{
			vmime::net::fetchAttributes attribs = vmime::net::fetchAttributes::IMPORTANCE;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("importance", "FETCH 42 BODY[HEADER.FIELDS (IMPORTANCE X-PRIORITY)]", cmd->getText());
		}

		// Any header attribute + full header should give RFC822.HEADER
		{
			vmime::net::fetchAttributes attribs;
			attribs.add(vmime::net::fetchAttributes::ENVELOPE);
			attribs.add(vmime::net::fetchAttributes::FULL_HEADER);

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("full-header", "FETCH 42 RFC822.HEADER", cmd->getText());
		}

		// Test custom header
		{
			vmime::net::fetchAttributes attribs;
			attribs.add("X-MyHeader");

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("custom-header", "FETCH 42 BODY[HEADER.FIELDS (x-myheader)]", cmd->getText());
		}

		// Test multiple flags
		{
			vmime::net::fetchAttributes attribs =
				vmime::net::fetchAttributes::UID | vmime::net::fetchAttributes::FLAGS;

			vmime::shared_ptr <IMAPCommand> cmd = IMAPUtils::buildFetchCommand(cnt, msgs, attribs);
			VASSERT_EQ("multiple", "FETCH 42 (FLAGS UID)", cmd->getText());
		}
	}

VMIME_TEST_SUITE_END
