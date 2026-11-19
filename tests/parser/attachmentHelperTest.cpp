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


VMIME_TEST_SUITE_BEGIN(attachmentHelperTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testAddAttachment1)
		VMIME_TEST(testAddAttachment2)
		VMIME_TEST(testAddAttachment3)
		VMIME_TEST(testIsBodyPartAnAttachment1)
		VMIME_TEST(testIsBodyPartAnAttachment2)
		VMIME_TEST(testIsBodyPartAnAttachment3)
		VMIME_TEST(testIsAttachment)
		VMIME_TEST(testGetAttachmentName)
		VMIME_TEST(testGetBodyPartAttachment)
		VMIME_TEST(testAddAttachmentMessage1)
		VMIME_TEST(testAddAttachmentMessage8bit)
		VMIME_TEST(testAddAttachmentMessageBinary)
		VMIME_TEST(testGetBodyPartAttachmentMessage)
	VMIME_TEST_LIST_END


	static vmime::string getStructure(const vmime::shared_ptr <vmime::bodyPart>& part) {

		vmime::shared_ptr <vmime::body> bdy = part->getBody();

		vmime::string res = part->getBody()->getContentType().generate();

		if (bdy->getPartCount() == 0) {
			return res;
		}

		res += "[";

		for (size_t i = 0 ; i < bdy->getPartCount() ; ++i) {

			vmime::shared_ptr <vmime::bodyPart> subPart = bdy->getPartAt(i);

			if (i != 0) {
				res += ",";
			}

			res += getStructure(subPart);
		}

		return res + "]";
	}

	static vmime::string extractBodyContents(
		const vmime::shared_ptr <const vmime::bodyPart>& part
	) {

		vmime::shared_ptr <const vmime::contentHandler> cth = part->getBody()->getContents();

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		cth->extract(os);

		return data;
	}

	void testAddAttachment1() {

		vmime::string data =
"Content-Type: text/plain\r\n"
"\r\n"
"The text\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		vmime::shared_ptr <vmime::attachment> att =
			vmime::make_shared <vmime::defaultAttachment>(
				vmime::make_shared <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg")
			);

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "The text\r\n", extractBodyContents(msg->getBody()->getPartAt(0)));
	}

	void testAddAttachment2() {

		vmime::string data =
"Content-Type: multipart/mixed; boundary=\"foo\"\r\n"
"\r\n"
"--foo\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"The text\r\n"
"--foo\r\n"
"Content-Type: application/octet-stream\r\n"
"\r\n"
"Blah\r\n"
"--foo--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		vmime::shared_ptr <vmime::attachment> att =
			vmime::make_shared <vmime::defaultAttachment>(
				vmime::make_shared <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg")
			);

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,application/octet-stream,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "The text", extractBodyContents(msg->getBody()->getPartAt(0)));
		VASSERT_EQ("3", "Blah", extractBodyContents(msg->getBody()->getPartAt(1)));
		VASSERT_EQ("4", "test", extractBodyContents(msg->getBody()->getPartAt(2)));
	}

	// Initial part is encoded
	void testAddAttachment3() {

		vmime::string data =
"Content-Type: text/plain\r\n"
"Content-Transfer-Encoding: base64\r\n"
"\r\n"
"TWVzc2FnZSBib2R5";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		vmime::shared_ptr <vmime::attachment> att =
			vmime::make_shared <vmime::defaultAttachment>(
				vmime::make_shared <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg")
			);

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "Message body", extractBodyContents(msg->getBody()->getPartAt(0)));
	}

	// Content-Disposition: attachment
	// No other field
	void testIsBodyPartAnAttachment1() {

		vmime::string data = "Content-Disposition: attachment\r\n\r\nFoo\r\n";

		vmime::shared_ptr <vmime::bodyPart> p = vmime::make_shared <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", true, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// No Content-Disposition field
	// Content-Type: multipart/* or text/*
	void testIsBodyPartAnAttachment2() {

		vmime::string data = "Content-Type: multipart/*\r\n\r\nFoo\r\n";

		vmime::shared_ptr <vmime::bodyPart> p = vmime::make_shared <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));

		data = "Content-Type: text/*\r\n\r\nFoo\r\n";

		p->parse(data);

		VASSERT_EQ("2", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// No Content-Disposition field
	void testIsBodyPartAnAttachment3() {

		vmime::string data = "Content-Type: application/octet-stream\r\n\r\nFoo\r\n";

		vmime::shared_ptr <vmime::bodyPart> p = vmime::make_shared <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", true, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// Content-Disposition: attachment
	// Content-Id field present
	void testIsBodyPartAnAttachment4() {

		vmime::string data = "Content-Disposition: attachment\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Content-Id: bar\r\n"
			"\r\nFoo\r\n";

		vmime::shared_ptr <vmime::bodyPart> p = vmime::make_shared <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// Header only, for the root part or not
	void testIsAttachment() {

		vmime::header hdr;
		hdr.parse(
			"Content-Type: image/png\r\n"
			"Content-Disposition: inline\r\n"
		);

		// An inline part may be an attachment, but not the root part
		VASSERT_EQ("1", true, vmime::attachmentHelper::isAttachment(hdr, false));
		VASSERT_EQ("2", false, vmime::attachmentHelper::isAttachment(hdr, true));

		// Inline object (with a Content-Id field)
		hdr.parse(
			"Content-Type: image/png\r\n"
			"Content-Disposition: inline\r\n"
			"Content-Id: <logo@vmime.org>\r\n"
		);

		VASSERT_EQ("3", false, vmime::attachmentHelper::isAttachment(hdr, false));
		VASSERT_EQ("4", true, vmime::attachmentHelper::isAttachment(
			hdr, false, vmime::attachmentHelper::INLINE_OBJECTS));
	}

	void testGetAttachmentName() {

		vmime::header hdr;

		// The "filename" parameter of Content-Disposition comes first
		hdr.parse(
			"Content-Type: application/pdf; name=\"type.pdf\"\r\n"
			"Content-Disposition: attachment; filename=\"disposition.pdf\"\r\n"
		);

		VASSERT_EQ("1", "disposition.pdf", vmime::attachmentHelper::getAttachmentName(hdr).getBuffer());

		// Then the "name" parameter of Content-Type (RFC-1341)
		hdr.parse(
			"Content-Type: application/pdf; name=\"type.pdf\"\r\n"
			"Content-Disposition: attachment\r\n"
		);

		VASSERT_EQ("2", "type.pdf", vmime::attachmentHelper::getAttachmentName(hdr).getBuffer());

		// No name
		hdr.parse("Content-Type: application/pdf\r\n");

		VASSERT_EQ("3", "", vmime::attachmentHelper::getAttachmentName(hdr).getBuffer());
	}

	void testGetBodyPartAttachment() {

		vmime::string data =
			"Content-Type: image/jpeg\r\n"
			"Content-Description: foobar\r\n"
			"Content-Transfer-Encoding: x-baz\r\n"
			"Content-Disposition: attachment; filename=\"foobar.baz\"\r\n"
			"\r\n"
			"Foo bar baz";

		vmime::shared_ptr <vmime::bodyPart> part = vmime::make_shared <vmime::bodyPart>();
		part->parse(data);

		vmime::shared_ptr <const vmime::attachment> att =
			vmime::attachmentHelper::getBodyPartAttachment(part);

		VASSERT_EQ("1", "image/jpeg", att->getType().generate());
		VASSERT_EQ("2", "foobar", att->getDescription().generate());
		VASSERT_EQ("3", "x-baz", att->getEncoding().generate());
		VASSERT_EQ("4", "foobar.baz", att->getName().generate());

		vmime::string attData;
		vmime::utility::outputStreamStringAdapter out(attData);
		att->getData()->extractRaw(out);  // 'x-baz' encoding not supported

		VASSERT_EQ("5", "Foo bar baz", attData);

		//VASSERT_EQ("6", part, att->getPart());
		VASSERT_EQ("6", part->generate(), vmime::dynamicCast <const vmime::component>(att->getPart())->generate());
		//VASSERT_EQ("7", part->getHeader(), att->getHeader());
		VASSERT_EQ("7", part->getHeader()->generate(), att->getHeader()->generate());
	}

	void testAddAttachmentMessage1() {

		const vmime::string data =
"Subject: Test message\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"Message body";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		const vmime::string attData =
"Subject: Attached message\r\n"
"Content-Type: text/plain\r\n"
"Content-Transfer-Encoding: base64\r\n"
"\r\n"
"QXR0YWNoZWQgbWVzc2FnZSBib2R5";

		vmime::shared_ptr <vmime::message> amsg = vmime::make_shared <vmime::message>();
		amsg->parse(attData);

		vmime::attachmentHelper::addAttachment(msg, amsg);

		VASSERT_EQ("1", "multipart/mixed[text/plain,message/rfc822]", getStructure(msg));
		VASSERT_EQ("2", "Message body", extractBodyContents(msg->getBody()->getPartAt(0)));

		// Ensure message has been encoded properly
		vmime::shared_ptr <const vmime::bodyPart> attPart = msg->getBody()->getPartAt(1);
		vmime::shared_ptr <const vmime::contentHandler> attCth = attPart->getBody()->getContents();

		vmime::string attDataOut;
		vmime::utility::outputStreamStringAdapter attDataOutOs(attDataOut);

		attCth->extract(attDataOutOs);

		vmime::shared_ptr <vmime::message> amsgOut = vmime::make_shared <vmime::message>();
		amsgOut->parse(attDataOut);

		vmime::shared_ptr <vmime::header> hdr = amsgOut->getHeader();

		VASSERT_EQ("3", "Attached message", hdr->Subject()->getValue <vmime::text>()->generate());
		VASSERT_EQ("4", "Attached message body", extractBodyContents(amsgOut));

		// A message/rfc822 part cannot be encoded: it is declared as 7-bit
		// as it only contains 7-bit data (RFC 2046, section 5.2.1)
		VASSERT_EQ("5", "7bit", attPart->getBody()->getEncoding().generate());
	}

	// A message/rfc822 part which contains 8-bit data is declared as 8-bit
	// (RFC 2046, section 5.2.1)
	void testAddAttachmentMessage8bit() {

		const vmime::string data =
"Subject: Test message\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"Message body";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		const vmime::string attData =
"Subject: Attached message\r\n"
"Content-Type: text/plain; charset=utf-8\r\n"
"Content-Transfer-Encoding: 8bit\r\n"
"\r\n"
"Gr\xc3\xbc\xc3\x9f" "e";

		vmime::shared_ptr <vmime::message> amsg = vmime::make_shared <vmime::message>();
		amsg->parse(attData);

		vmime::attachmentHelper::addAttachment(msg, amsg);

		VASSERT_EQ("1", "multipart/mixed[text/plain,message/rfc822]", getStructure(msg));

		vmime::shared_ptr <const vmime::bodyPart> attPart = msg->getBody()->getPartAt(1);

		VASSERT_EQ("2", "8bit", attPart->getBody()->getEncoding().generate());
		VASSERT_EQ(
			"3", "8bit",
			attPart->getHeader()->findField(vmime::fields::CONTENT_TRANSFER_ENCODING)->getValue()->generate()
		);
	}

	// A message/rfc822 part which contains binary data is declared as binary
	// (RFC 2046, section 5.2.1), and its data is not altered
	void testAddAttachmentMessageBinary() {

		const vmime::string data =
"Subject: Test message\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"Message body";

		// Binary part (NUL, LF alone)
		const vmime::string binaryData("A\0B\nC", 5);

		const vmime::string attData =
"Subject: Attached message\r\n"
"Content-Type: application/octet-stream\r\n"
"Content-Transfer-Encoding: binary\r\n"
"\r\n" + binaryData;

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		vmime::shared_ptr <vmime::message> amsg = vmime::make_shared <vmime::message>();
		amsg->parse(attData);

		vmime::attachmentHelper::addAttachment(msg, amsg);

		VASSERT_EQ("1.1", "binary", msg->getBody()->getPartAt(1)->getBody()->getEncoding().generate());
		VASSERT("1.2", msg->generate().find("\r\n\r\n" + binaryData + "\r\n--") != vmime::string::npos);

		// Line longer than 998 octets
		const vmime::string longLineData =
"Subject: Attached message\r\n"
"Content-Type: text/plain\r\n"
"\r\n" + vmime::string(999, 'x') + "\r\n";

		vmime::shared_ptr <vmime::message> msg2 = vmime::make_shared <vmime::message>();
		msg2->parse(data);

		vmime::shared_ptr <vmime::message> amsg2 = vmime::make_shared <vmime::message>();
		amsg2->parse(longLineData);

		vmime::attachmentHelper::addAttachment(msg2, amsg2);

		VASSERT_EQ("2.1", "binary", msg2->getBody()->getPartAt(1)->getBody()->getEncoding().generate());
	}

	void testGetBodyPartAttachmentMessage() {

		const vmime::string data =
"Subject: Test message\r\n"
"Content-Type: multipart/mixed; boundary=\"foo\"\r\n"
"\r\n"
"--foo\r\n"
"Content-Type: message/rfc822\r\n"
"\r\n"
"Subject: Attached message\r\n"
"\r\n"
"Attached message body\r\n"
"--foo\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"FooBar\r\n"
"--foo--\r\n";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(data);

		VASSERT_EQ("0", 2, msg->getBody()->getPartCount());

		vmime::shared_ptr <const vmime::attachment> att =
			vmime::attachmentHelper::getBodyPartAttachment(msg->getBody()->getPartAt(0));

		VASSERT("1", att != NULL);

		vmime::shared_ptr <const vmime::messageAttachment> msgAtt =
			vmime::dynamicCast <const vmime::messageAttachment>(att);

		VASSERT("2", msgAtt != NULL);

		vmime::shared_ptr <vmime::message> amsg = msgAtt->getMessage();
		vmime::shared_ptr <vmime::header> hdr = amsg->getHeader();

		VASSERT_EQ("3", "Attached message", hdr->Subject()->getValue <vmime::text>()->generate());
		VASSERT_EQ("4", "Attached message body", extractBodyContents(amsg));
	}

VMIME_TEST_SUITE_END
