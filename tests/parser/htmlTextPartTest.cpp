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
#include "vmime/htmlTextPart.hpp"


#define VMIME_TEST_SUITE         htmlTextPartTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseText)
		VMIME_TEST(testParseEmbeddedObjectsCID)
		VMIME_TEST(testParseEmbeddedObjectsLocation)
	VMIME_TEST_LIST_END


	static const vmime::string extractContent
		(vmime::ref <const vmime::contentHandler> cth)
	{
		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth->extract(osa);

		return oss.str();
	}


	void testParseText()
	{
		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/alternative; boundary=\"LEVEL1\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/plain; charset=\"x-ch1\"\r\n"
"\r\n"
"Plain text part\r\n"
"--LEVEL1\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL2\"\r\n"
"\r\n"
"--LEVEL2\r\n"
"Content-Type: text/html; charset=\"x-ch2\"\r\n"
"\r\n"
"<img src=\"cid:image@test\"/>\r\n"
"--LEVEL2\r\n"
"Content-Type: image/png; name=\"image.png\"\r\n"
"Content-Disposition: inline; filename=\"image.png\"\r\n"
"Content-ID: <image@test>\r\n"
"\r\n"
"Image\r\n"
"--LEVEL2--\r\n"
"\r\n"
"--LEVEL1--\r\n"
"";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(msgString);

		// Sanity checks
		VASSERT_EQ("part-count1", 2, msg->getBody()->getPartCount());
		VASSERT_EQ("part-count2", 2, msg->getBody()->getPartAt(1)->getBody()->getPartCount());

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg->getBody()->getPartAt(1),
			msg->getBody()->getPartAt(1)->getBody()->getPartAt(0));

		VASSERT_EQ("plain", "Plain text part", extractContent(htmlPart.getPlainText()));
		VASSERT_EQ("html", "<img src=\"cid:image@test\"/>", extractContent(htmlPart.getText()));

		// Should return the charset of the HTML part
		VASSERT_EQ("charset", "x-ch2", htmlPart.getCharset().generate());
	}

	/** Test parsing of embedded objects by CID (Content-Id).
	  */
	void testParseEmbeddedObjectsCID()
	{
		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/alternative; boundary=\"LEVEL1\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/plain; charset=\"x-ch1\"\r\n"
"\r\n"
"Plain text part\r\n"
"--LEVEL1\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL2\"\r\n"
"\r\n"
"--LEVEL2\r\n"  // one embedded object before...
"Content-Type: image/png; name=\"image1.png\"\r\n"
"Content-Disposition: inline; filename=\"image1.png\"\r\n"
"Content-ID: <image1@test>\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL2\r\n"  // ...the actual text part...
"Content-Type: text/html; charset=\"x-ch2\"\r\n"
"\r\n"
"<img src=\"cid:image1@test\"/>\r\n"
"<img src=\"CID:image2@test\"/>\r\n"
"--LEVEL2\r\n"  // ...and one after
"Content-Type: image/jpeg; name=\"image2.jpg\"\r\n"
"Content-Disposition: inline; filename=\"image2.jpg\"\r\n"
"Content-ID: <image2@test>\r\n"
"\r\n"
"Image2\r\n"
"--LEVEL2--\r\n"
"\r\n"
"--LEVEL1--\r\n"
"";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(msgString);

		// Sanity checks
		VASSERT_EQ("part-count1", 2, msg->getBody()->getPartCount());
		VASSERT_EQ("part-count2", 3, msg->getBody()->getPartAt(1)->getBody()->getPartCount());

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg->getBody()->getPartAt(1),
			msg->getBody()->getPartAt(1)->getBody()->getPartAt(1));

		// Two embedded objects should be found.
		// BUGFIX: "CID:" prefix is not case-sensitive.
		VASSERT_EQ("count", 2, htmlPart.getObjectCount());

		// Ensure the right objects have been found.
		VASSERT_EQ("has-obj1", true, htmlPart.hasObject("image1@test"));
		VASSERT_EQ("has-obj2", true, htmlPart.hasObject("image2@test"));

		// hasObject() should also work with prefixes
		VASSERT_EQ("has-obj1-pre", true, htmlPart.hasObject("CID:image1@test"));
		VASSERT_EQ("has-obj2-pre", true, htmlPart.hasObject("cid:image2@test"));

		// Check data in objects
		vmime::ref <const vmime::htmlTextPart::embeddedObject> obj;

		obj = htmlPart.findObject("image1@test");

		VASSERT_EQ("id-obj1", "image1@test", obj->getId());
		VASSERT_EQ("data-obj1", "Image1", extractContent(obj->getData()));
		VASSERT_EQ("type-obj1", "image/png", obj->getType().generate());

		obj = htmlPart.findObject("image2@test");

		VASSERT_EQ("id-obj2", "image2@test", obj->getId());
		VASSERT_EQ("data-obj2", "Image2", extractContent(obj->getData()));
		VASSERT_EQ("type-obj2", "image/jpeg", obj->getType().generate());
	}

	/** Test parsing of embedded objects by location.
	  */
	void testParseEmbeddedObjectsLocation()
	{
		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/alternative; boundary=\"LEVEL1\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/plain; charset=\"x-ch1\"\r\n"
"\r\n"
"Plain text part\r\n"
"--LEVEL1\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL2\"\r\n"
"\r\n"
"--LEVEL2\r\n"
"Content-Type: image/png; name=\"image1.png\"\r\n"
"Content-Location: http://www.vmime.org/test/image1.png\r\n"
"Content-Disposition: inline; filename=\"image1.png\"\r\n"
"Content-Id: <image1@test>\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL2\r\n"
"Content-Type: text/html; charset=\"x-ch2\"\r\n"
"\r\n"
"<img src=\"http://www.vmime.org/test/image1.png\"/>\r\n"
"--LEVEL2--\r\n"
"\r\n"
"--LEVEL1--\r\n"
"";

		vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
		msg->parse(msgString);

		// Sanity checks
		VASSERT_EQ("part-count1", 2, msg->getBody()->getPartCount());
		VASSERT_EQ("part-count2", 2, msg->getBody()->getPartAt(1)->getBody()->getPartCount());

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg->getBody()->getPartAt(1),
			msg->getBody()->getPartAt(1)->getBody()->getPartAt(1));

		// Only one embedded object
		VASSERT_EQ("count", 1, htmlPart.getObjectCount());

		// Should work only with Content-Location as the Content-Id is
		// not referenced in the HTML contents
		VASSERT_EQ("has-obj-loc", true, htmlPart.hasObject("http://www.vmime.org/test/image1.png"));
		VASSERT_EQ("has-obj-cid", false, htmlPart.hasObject("image1@test"));

		// Check data
		vmime::ref <const vmime::htmlTextPart::embeddedObject> obj;

		obj = htmlPart.findObject("http://www.vmime.org/test/image1.png");

		VASSERT_EQ("id-obj", "http://www.vmime.org/test/image1.png", obj->getId());
		VASSERT_EQ("data-obj", "Image1", extractContent(obj->getData()));
		VASSERT_EQ("type-obj", "image/png", obj->getType().generate());
	}

	// TODO: test generation of text parts

VMIME_TEST_SUITE_END

