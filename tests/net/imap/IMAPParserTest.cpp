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

#include "vmime/net/imap/IMAPTag.hpp"
#include "vmime/net/imap/IMAPParser.hpp"


VMIME_TEST_SUITE_BEGIN(IMAPParserTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testExtraSpaceInCapaResponse)
		VMIME_TEST(testContinueReqWithoutSpace)
		VMIME_TEST(testNILValueInBodyFldEnc)
		VMIME_TEST(testFETCHResponse_optional_body_fld_lang)
		VMIME_TEST(testFETCHBodyStructure_NIL_body_fld_param_value)
		VMIME_TEST(testFETCHBodyStructure_empty_body_fld_param_instead_of_NIL)
		VMIME_TEST(testPipelining)
		VMIME_TEST(testStarFlagWithoutBackslash)
	VMIME_TEST_LIST_END


	// For Apple iCloud IMAP server
	void testExtraSpaceInCapaResponse() {

		const char* resp =
			"* CAPABILITY IMAP4rev1 AUTH=ATOKEN AUTH=PLAIN \r\n"  // extra space at end
			"a001 OK Capability completed.\r\n";

		// Strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(true);

			VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
		}

		// Non-strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(false);

			VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));
		}
	}

	// For Apple iCloud/Exchange IMAP server
	void testContinueReqWithoutSpace() {

		// continue_req   ::= "+" SPACE (resp_text / base64)
		//
		// Some servers do not send SPACE when response text is empty.
		// IMAP parser should allow this in non-strict mode.
		//
		// Eg:
		//
		//   C: a002 AUTHENTICATE xxx[CR][LF]
		//   S: +[CR][LF]

		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
				vmime::make_shared <vmime::net::imap::IMAPTag>();

		socket->localSend("+\r\n");

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		parser->setStrict(false);
		VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));

		++(*tag);

		socket->localSend("+\r\n");

		parser->setStrict(true);
		VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
	}

	// When an IMAP4 client sends a FETCH (bodystructure) request to a server
	// that is running the Exchange Server 2007 IMAP4 service, a corrupted
	// response is sent as a reply
	// --> http://support.microsoft.com/kb/975918/en-us
	void testNILValueInBodyFldEnc() {

		const char* resp = "* 7970 FETCH (UID 8036 FLAGS () BODYSTRUCTURE (\"text\" \"html\" (\"charset\" \"utf-8\") NIL NIL NIL 175501 1651 NIL NIL NIL NIL) RFC822.HEADER {3}\r\nx\r\n)\r\na001 OK FETCH complete\r\n";

		// Strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(true);

			VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
		}

		// Non-strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(false);

			VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));
		}
	}

	// "body_fld_lang" is optional after "body_fld_dsp" in "body_ext_mpart" (Yahoo)
	void testFETCHResponse_optional_body_fld_lang() {

		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		const char* resp = "* 1 FETCH (UID 7 RFC822.SIZE 694142 BODYSTRUCTURE (((\"text\" \"plain\" (\"charset\" \"utf-8\") NIL NIL \"7bit\" 0 0 NIL NIL NIL NIL)(\"text\" \"html\" (\"charset\" \"utf-8\") NIL NIL \"7bit\" 193 0 NIL NIL NIL NIL) \"alternative\" (\"boundary\" \"----=_Part_536_109505883.1410847112666\") NIL)(\"image\" \"jpeg\" NIL \"<4db20d0e-e9f8-729b-aaf7-688b5956d0bc@yahoo.com>\" NIL \"base64\" 351784 NIL (\"attachment\" (\"name\" \"att2\" \"filename\" \"9.jpg\")) NIL NIL)(\"image\" \"jpeg\" NIL \"<542417d7-c0ed-db72-f9fc-d9ab2c7e0a6f@yahoo.com>\" NIL \"base64\" 337676 NIL (\"attachment\" (\"name\" \"att3\" \"filename\" \"10.jpg\")) NIL NIL) \"mixed\" (\"boundary\" \"----=_Part_537_1371134700.1410847112668\") NIL) RFC822.HEADER {3}\r\nx\r\n)\r\na001 OK FETCH complete\r\n";

		socket->localSend(resp);

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		VASSERT_NO_THROW("parse", parser->readResponse(*tag));
	}

	// Support for NIL boundary, for mail.ru IMAP server:
	// https://www.ietf.org/mail-archive/web/imapext/current/msg05442.html
	void testFETCHBodyStructure_NIL_body_fld_param_value() {

		// ...("boundary" NIL)))... is an invalid syntax for a "body_fld_param_item"
		const char* resp = "* 1 FETCH (BODYSTRUCTURE ((\"text\" \"plain\" (\"charset\" \"utf-8\") NIL NIL \"8bit\" 536 0 NIL NIL NIL NIL)(\"text\" \"html\" (\"charset\" \"utf-8\") NIL NIL \"8bit\" 7130 0 NIL NIL NIL NIL) \"alternative\" (\"boundary\" NIL)))\r\na001 OK FETCH complete\r\n";

		// Strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(true);

			VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
		}

		// Non-strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(false);

			VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));
		}
	}

	void testFETCHBodyStructure_empty_body_fld_param_instead_of_NIL() {

		const char* resp = "* 1 FETCH (BODYSTRUCTURE ((\"text\" \"html\" (\"charset\" \"cp1251\") NIL NIL \"base64\" 84056 0 NIL (\"inline\" NIL) NIL NIL)(\"image\" \"gif\" () \"25b2b55b5d97f04e9ea939fe32a46a65.gif\" NIL \"base64\" 20776 NIL (\"inline\" NIL) NIL NIL) \"related\" (\"boundary\" NIL)))\r\na001 OK FETCH complete\r\n";

		// Strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(true);

			VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
		}

		// Non-strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();
			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(vmime::make_shared <testTimeoutHandler>());
			parser->setStrict(false);

			VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));
		}
	}

	// Test pipelined and out-of-order replies
	void testPipelining() {

		/*
		[C] a001 SELECT "INBOX"
		[C] a002 UID FETCH 42 (INTERNALDATE)
		[C] a003 UID FETCH 43 (INTERNALDATE)
		[S] * NO Error for a001
		[S] a001 NO Access denied
		[S] * NO Error for a003
		[S] a003 BAD No mailbox selected
		[S] * NO Error for a002
		[S] a002 BAD No mailbox selected
		*/

		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::net::imap::IMAPTag tag1;  // a001
		vmime::net::imap::IMAPTag tag2(tag1);  // a002
		++tag2;
		vmime::net::imap::IMAPTag tag3(tag2);  // a003
		++tag3;

		socket->localSend(
			"* NO Error for a001\r\n"
			"a001 NO Access denied\r\n"
			"* NO Error for a003\r\n"
			"a003 BAD No mailbox selected a003\r\n"
			"* NO Error for a002\r\n"
			"a002 BAD No mailbox selected a002\r\n"
		);

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		// Test response a001
		vmime::scoped_ptr <vmime::net::imap::IMAPParser::response> resp1(parser->readResponse(tag1, /* literalHandler */ NULL));
		VASSERT("a001 response", resp1);
		VASSERT("a001 response_done", resp1->response_done);
		VASSERT_EQ("a001 response tag", "a001", resp1->response_done->response_tagged->tag->tagString);
		VASSERT_EQ("a001 response text", "Access denied", resp1->response_done->response_tagged->resp_cond_state->resp_text->text);
		VASSERT_EQ("a001 resp_data.size()", 1, resp1->continue_req_or_response_data.size());
		VASSERT("a001 resp_data[0]", resp1->continue_req_or_response_data[0]->response_data);
		VASSERT("a001 resp_cond_state", resp1->continue_req_or_response_data[0]->response_data->resp_cond_state);
		VASSERT_EQ("a001 resp_cond_state.text", "Error for a001", resp1->continue_req_or_response_data[0]->response_data->resp_cond_state->resp_text->text);
		VASSERT_EQ("a001 resp_cond_state.status", vmime::net::imap::IMAPParser::resp_cond_state::NO, resp1->continue_req_or_response_data[0]->response_data->resp_cond_state->status);

		// Test response a002
		vmime::scoped_ptr <vmime::net::imap::IMAPParser::response> resp2(parser->readResponse(tag2, /* literalHandler */ NULL));
		VASSERT("a002 response", resp2);
		VASSERT("a002 response_done", resp2->response_done);
		VASSERT_EQ("a002 response tag", "a002", resp2->response_done->response_tagged->tag->tagString);
		VASSERT_EQ("a002 response text", "No mailbox selected a002", resp2->response_done->response_tagged->resp_cond_state->resp_text->text);
		VASSERT_EQ("a002 resp_data.size()", 1, resp2->continue_req_or_response_data.size());
		VASSERT("a002 resp_data[0]", resp2->continue_req_or_response_data[0]->response_data);
		VASSERT("a002 resp_cond_state", resp2->continue_req_or_response_data[0]->response_data->resp_cond_state);
		VASSERT_EQ("a002 resp_cond_state.text", "Error for a002", resp2->continue_req_or_response_data[0]->response_data->resp_cond_state->resp_text->text);
		VASSERT_EQ("a002 resp_cond_state.status", vmime::net::imap::IMAPParser::resp_cond_state::NO, resp2->continue_req_or_response_data[0]->response_data->resp_cond_state->status);

		// Test response a003
		vmime::scoped_ptr <vmime::net::imap::IMAPParser::response> resp3(parser->readResponse(tag3, /* literalHandler */ NULL));
		VASSERT("a003 response", resp3);
		VASSERT("a003 response_done", resp3->response_done);
		VASSERT_EQ("a003 response tag", "a003", resp3->response_done->response_tagged->tag->tagString);
		VASSERT_EQ("a003 response text", "No mailbox selected a003", resp3->response_done->response_tagged->resp_cond_state->resp_text->text);
		VASSERT_EQ("a003 resp_data.size()", 1, resp3->continue_req_or_response_data.size());
		VASSERT("a003 resp_data[0]", resp3->continue_req_or_response_data[0]->response_data);
		VASSERT("a003 resp_cond_state", resp3->continue_req_or_response_data[0]->response_data->resp_cond_state);
		VASSERT_EQ("a003 resp_cond_state.text", "Error for a003", resp3->continue_req_or_response_data[0]->response_data->resp_cond_state->resp_text->text);
		VASSERT_EQ("a003 resp_cond_state.status", vmime::net::imap::IMAPParser::resp_cond_state::NO, resp3->continue_req_or_response_data[0]->response_data->resp_cond_state->status);
	}

	// Some IMAP servers return "*" instead of "\*" in PERMANENTFLAGS
	void testStarFlagWithoutBackslash() {

		const char* resp =
			"* OK [PERMANENTFLAGS (Answered Flagged Deleted Seen Draft *)] Flags permitted.\r\n"
			"a001 OK Completed.\r\n";

		// Strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			auto toh = vmime::make_shared <testTimeoutHandler>();

			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(toh);
			parser->setStrict(true);

			VASSERT_THROW("strict mode", parser->readResponse(*tag), vmime::exceptions::invalid_response);
		}

		// Non-strict mode
		{
			auto socket = vmime::make_shared <testSocket>();
			auto toh = vmime::make_shared <testTimeoutHandler>();

			auto tag = vmime::make_shared <vmime::net::imap::IMAPTag>();

			socket->localSend(resp);

			auto parser = vmime::make_shared <vmime::net::imap::IMAPParser>();

			parser->setSocket(socket);
			parser->setTimeoutHandler(toh);
			parser->setStrict(false);

			VASSERT_NO_THROW("non-strict mode", parser->readResponse(*tag));
		}
	}

VMIME_TEST_SUITE_END
