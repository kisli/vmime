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


#define VMIME_TEST_SUITE         attachmentHelperTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testAddAttachment1)
		VMIME_TEST(testAddAttachment2)
		VMIME_TEST(testAddAttachment3)
		VMIME_TEST(testIsBodyPartAnAttachment1)
		VMIME_TEST(testIsBodyPartAnAttachment2)
		VMIME_TEST(testIsBodyPartAnAttachment3)
		VMIME_TEST(testGetBodyPartAttachment)
		VMIME_TEST(testAddAttachmentMessage1)
		VMIME_TEST(testGetBodyPartAttachmentMessage)
	VMIME_TEST_LIST_END


	static const vmime::string getStructure(vmime::ref <vmime::bodyPart> part)
	{
		vmime::ref <vmime::body> bdy = part->getBody();

		vmime::string res = part->getBody()->getContentType().generate();

		if (bdy->getPartCount() == 0)
			return res;

		res += "[";

		for (int i = 0 ; i < bdy->getPartCount() ; ++i)
		{
			vmime::ref <vmime::bodyPart> subPart = bdy->getPartAt(i);

			if (i != 0)
				res += ",";

			res += getStructure(subPart);
		}

		return res + "]";
	}

	static const vmime::string extractBodyContents(vmime::ref <const vmime::bodyPart> part)
	{
		vmime::ref <const vmime::contentHandler> cth = part->getBody()->getContents();

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		cth->extract(os);

		return data;
	}

	void testAddAttachment1()
	{
		vmime::string data =
"Content-Type: text/plain\r\n"
"\r\n"
"The text\r\n"
"";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(data);

		vmime::ref <vmime::attachment> att = vmime::create <vmime::defaultAttachment>
			(vmime::create <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg"));

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "The text\r\n", extractBodyContents(msg->getBody()->getPartAt(0)));
	}

	void testAddAttachment2()
	{
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

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(data);

		vmime::ref <vmime::attachment> att = vmime::create <vmime::defaultAttachment>
			(vmime::create <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg"));

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,application/octet-stream,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "The text", extractBodyContents(msg->getBody()->getPartAt(0)));
		VASSERT_EQ("3", "Blah", extractBodyContents(msg->getBody()->getPartAt(1)));
		VASSERT_EQ("4", "test", extractBodyContents(msg->getBody()->getPartAt(2)));
	}

	// Initial part is encoded
	void testAddAttachment3()
	{
		vmime::string data =
"Content-Type: text/plain\r\n"
"Content-Transfer-Encoding: base64\r\n"
"\r\n"
"TWVzc2FnZSBib2R5";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(data);

		vmime::ref <vmime::attachment> att = vmime::create <vmime::defaultAttachment>
			(vmime::create <vmime::stringContentHandler>("test"),
				vmime::mediaType("image/jpeg"));

		vmime::attachmentHelper::addAttachment(msg, att);

		VASSERT_EQ("1", "multipart/mixed[text/plain,image/jpeg]", getStructure(msg));
		VASSERT_EQ("2", "Message body", extractBodyContents(msg->getBody()->getPartAt(0)));
	}

	// Content-Disposition: attachment
	// No other field
	void testIsBodyPartAnAttachment1()
	{
		vmime::string data = "Content-Disposition: attachment\r\n\r\nFoo\r\n";

		vmime::ref <vmime::bodyPart> p = vmime::create <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", true, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// No Content-Disposition field
	// Content-Type: multipart/* or text/*
	void testIsBodyPartAnAttachment2()
	{
		vmime::string data = "Content-Type: multipart/*\r\n\r\nFoo\r\n";

		vmime::ref <vmime::bodyPart> p = vmime::create <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));

		data = "Content-Type: text/*\r\n\r\nFoo\r\n";

		p->parse(data);

		VASSERT_EQ("2", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// No Content-Disposition field
	void testIsBodyPartAnAttachment3()
	{
		vmime::string data = "Content-Type: application/octet-stream\r\n\r\nFoo\r\n";

		vmime::ref <vmime::bodyPart> p = vmime::create <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", true, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	// Content-Disposition: attachment
	// Content-Id field present
	void testIsBodyPartAnAttachment4()
	{
		vmime::string data = "Content-Disposition: attachment\r\n"
			"Content-Type: application/octet-stream\r\n"
			"Content-Id: bar\r\n"
			"\r\nFoo\r\n";

		vmime::ref <vmime::bodyPart> p = vmime::create <vmime::bodyPart>();
		p->parse(data);

		VASSERT_EQ("1", false, vmime::attachmentHelper::isBodyPartAnAttachment(p));
	}

	void testGetBodyPartAttachment()
	{
		vmime::string data =
			"Content-Type: image/jpeg\r\n"
			"Content-Description: foobar\r\n"
			"Content-Transfer-Encoding: x-baz\r\n"
			"Content-Disposition: attachment; filename=\"foobar.baz\"\r\n"
			"\r\n"
			"Foo bar baz";

		vmime::ref <vmime::bodyPart> part = vmime::create <vmime::bodyPart>();
		part->parse(data);

		vmime::ref <const vmime::attachment> att =
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
		VASSERT_EQ("6", part->generate(), att->getPart().dynamicCast <const vmime::component>()->generate());
		//VASSERT_EQ("7", part->getHeader(), att->getHeader());
		VASSERT_EQ("7", part->getHeader()->generate(), att->getHeader()->generate());
	}

	void testAddAttachmentMessage1()
	{
		const vmime::string data =
"Subject: Test message\r\n"
"Content-Type: text/plain\r\n"
"\r\n"
"Message body";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(data);

		const vmime::string attData =
"Subject: Attached message\r\n"
"Content-Type: text/plain\r\n"
"Content-Transfer-Encoding: base64\r\n"
"\r\n"
"QXR0YWNoZWQgbWVzc2FnZSBib2R5";

		vmime::ref <vmime::message> amsg = vmime::create <vmime::message>();
		amsg->parse(attData);

		vmime::attachmentHelper::addAttachment(msg, amsg);

		VASSERT_EQ("1", "multipart/mixed[text/plain,message/rfc822]", getStructure(msg));
		VASSERT_EQ("2", "Message body", extractBodyContents(msg->getBody()->getPartAt(0)));

		// Ensure message has been encoded properly
		vmime::ref <const vmime::bodyPart> attPart = msg->getBody()->getPartAt(1);
		vmime::ref <const vmime::contentHandler> attCth = attPart->getBody()->getContents();

		vmime::string attDataOut;
		vmime::utility::outputStreamStringAdapter attDataOutOs(attDataOut);

		attCth->extract(attDataOutOs);

		vmime::ref <vmime::message> amsgOut = vmime::create <vmime::message>();
		amsgOut->parse(attDataOut);

		vmime::ref <vmime::header> hdr = amsgOut->getHeader();

		VASSERT_EQ("3", "Attached message", hdr->Subject()->getValue().dynamicCast <vmime::text>()->generate());
		VASSERT_EQ("4", "Attached message body", extractBodyContents(amsgOut));
	}

	void testGetBodyPartAttachmentMessage()
	{
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

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(data);

		VASSERT_EQ("0", 2, msg->getBody()->getPartCount());

		vmime::ref <const vmime::attachment> att = vmime::attachmentHelper::
			getBodyPartAttachment(msg->getBody()->getPartAt(0));

		VASSERT("1", att != NULL);

		vmime::ref <const vmime::messageAttachment> msgAtt =
			att.dynamicCast <const vmime::messageAttachment>();

		VASSERT("2", msgAtt != NULL);

		vmime::ref <vmime::message> amsg = msgAtt->getMessage();
		vmime::ref <vmime::header> hdr = amsg->getHeader();

		VASSERT_EQ("3", "Attached message", hdr->Subject()->getValue().dynamicCast <vmime::text>()->generate());
		VASSERT_EQ("4", "Attached message body", extractBodyContents(amsg));
	}

VMIME_TEST_SUITE_END

