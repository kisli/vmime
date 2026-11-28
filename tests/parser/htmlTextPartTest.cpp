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
#include "vmime/htmlTextPart.hpp"
#include "vmime/contentTypeField.hpp"


VMIME_TEST_SUITE_BEGIN(htmlTextPartTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseText)
		VMIME_TEST(testParseEmbeddedObjectsCID)
		VMIME_TEST(testParseEmbeddedObjectsLocation)
		VMIME_TEST(testParseEmbeddedObjectsLocationRelative)
		VMIME_TEST(testParseEmbeddedObjectsLocationNoBase)
		VMIME_TEST(testParseEmbeddedObjectsLocationHTMLBase)
		VMIME_TEST(testParseEmbeddedObjectsLocationCID)
		VMIME_TEST(testParseEmbeddedObjectsLocationNested)
		VMIME_TEST(testParseEmbeddedObjectsLocationEncoded)
		VMIME_TEST(testParseEmbeddedObjectsLocationNotRelated)
		VMIME_TEST(testGenerateEmbeddedObjectsLocation)
		VMIME_TEST(testGenerateEmbeddedObjectsLongLocation)
		VMIME_TEST(testGenerateParsedEmbeddedObjects)
		VMIME_TEST(testAddObjectInvalidLocation)
	VMIME_TEST_LIST_END


	static vmime::string extractContent(
		const vmime::shared_ptr <const vmime::contentHandler>& cth
	) {

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth->extract(osa);

		return oss.str();
	}


	void testParseText() {

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

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
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
	void testParseEmbeddedObjectsCID() {

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

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		// Sanity checks
		VASSERT_EQ("part-count1", 2, msg->getBody()->getPartCount());
		VASSERT_EQ("part-count2", 3, msg->getBody()->getPartAt(1)->getBody()->getPartCount());

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(
			msg, msg->getBody()->getPartAt(1),
			msg->getBody()->getPartAt(1)->getBody()->getPartAt(1)
		);

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
		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> obj;

		obj = htmlPart.findObject("image1@test");

		VASSERT_EQ("ref-type1", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_ID, obj->getReferenceType());
		VASSERT_EQ("id-obj1", "image1@test", obj->getId());
		VASSERT_EQ("data-obj1", "Image1", extractContent(obj->getData()));
		VASSERT_EQ("type-obj1", "image/png", obj->getType().generate());

		obj = htmlPart.findObject("image2@test");

		VASSERT_EQ("ref-type2", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_ID, obj->getReferenceType());
		VASSERT_EQ("id-obj2", "image2@test", obj->getId());
		VASSERT_EQ("data-obj2", "Image2", extractContent(obj->getData()));
		VASSERT_EQ("type-obj2", "image/jpeg", obj->getType().generate());
	}

	/** Test parsing of embedded objects by location.
	  */
	void testParseEmbeddedObjectsLocation() {

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

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		// Sanity checks
		VASSERT_EQ("part-count1", 2, msg->getBody()->getPartCount());
		VASSERT_EQ("part-count2", 2, msg->getBody()->getPartAt(1)->getBody()->getPartCount());

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(
			msg, msg->getBody()->getPartAt(1),
			msg->getBody()->getPartAt(1)->getBody()->getPartAt(1)
		);

		// Only one embedded object
		VASSERT_EQ("count", 1, htmlPart.getObjectCount());

		// Should work only with Content-Location as the Content-Id is
		// not referenced in the HTML contents
		VASSERT_EQ("has-obj-loc", true, htmlPart.hasObject("http://www.vmime.org/test/image1.png"));
		VASSERT_EQ("has-obj-cid", false, htmlPart.hasObject("image1@test"));

		// Check data
		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> obj;

		obj = htmlPart.findObject("http://www.vmime.org/test/image1.png");

		VASSERT_EQ("ref-type", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_LOCATION, obj->getReferenceType());
		VASSERT_EQ("id-obj", "http://www.vmime.org/test/image1.png", obj->getId());
		VASSERT_EQ("data-obj", "Image1", extractContent(obj->getData()));
		VASSERT_EQ("type-obj", "image/png", obj->getType().generate());
	}

	/** Test relative URIs, resolved against the Content-Location of the
	  * multipart/related part (RFC 2557, section 9.3).
	  */
	void testParseEmbeddedObjectsLocationRelative() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Location: http://www.vmime.org/test/\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"images/image1.png\"/>\r\n"
"<img src=\"images/image2.png\"/>\r\n"
"<img src=\"images/image3.png\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location:\r\n"
"    http://www.vmime.org/test/images/image1.png\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: images/image2.png\r\n"
"\r\n"
"Image2\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/test/ima\r\n"
" ges/image3.png\r\n"
"\r\n"
"Image3\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("count", 3, htmlPart.getObjectCount());

		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> obj;

		// Absolute Content-Location
		obj = htmlPart.findObject("images/image1.png");

		VASSERT("obj1", obj);
		VASSERT_EQ("ref-type1", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_LOCATION, obj->getReferenceType());
		VASSERT_EQ("id-obj1", "http://www.vmime.org/test/images/image1.png", obj->getId());
		VASSERT_EQ("data-obj1", "Image1", extractContent(obj->getData()));

		// Relative Content-Location: the identifier is the field value
		obj = htmlPart.findObject("http://www.vmime.org/test/images/image2.png");

		VASSERT("obj2", obj);
		VASSERT_EQ("id-obj2", "images/image2.png", obj->getId());
		VASSERT_EQ("data-obj2", "Image2", extractContent(obj->getData()));

		// Folded Content-Location (RFC 2557, section 4.4.3)
		obj = htmlPart.findObject("images/image3.png");

		VASSERT("obj3", obj);
		VASSERT_EQ("id-obj3", "http://www.vmime.org/test/images/image3.png", obj->getId());
		VASSERT_EQ("data-obj3", "Image3", extractContent(obj->getData()));

		// URIs are compared once resolved
		VASSERT_EQ("dot-segments", true, htmlPart.hasObject("./images/../images/image1.png"));
		VASSERT_EQ("other-dir", false, htmlPart.hasObject("image1.png"));
	}

	/** Test relative URIs without base URI (RFC 2557, section 9.4).
	  */
	void testParseEmbeddedObjectsLocationNoBase() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"image.png\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: image.png\r\n"
"\r\n"
"Image\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("count", 1, htmlPart.getObjectCount());
		VASSERT_EQ("id", "image.png", htmlPart.getObjectAt(0)->getId());

		VASSERT_EQ("relative", true, htmlPart.hasObject("image.png"));
		VASSERT_EQ("this-message", true, htmlPart.hasObject("thismessage:/image.png"));
		VASSERT_EQ("absolute", false, htmlPart.hasObject("http://www.vmime.org/image.png"));
	}

	/** Test the BASE element of the HTML text, which comes before the
	  * Content-Location fields (RFC 2557, section 5).
	  */
	void testParseEmbeddedObjectsLocationHTMLBase() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Location: http://www.vmime.org/\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<html><head><basefont size=\"3\"><BASE TARGET=_top HREF='test/'></head>\r\n"
"<body><img src=\"image.png\"/></body></html>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/test/image.png\r\n"
"\r\n"
"Image\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		// The relative base URI is resolved against the Content-Location
		VASSERT_EQ("base", true, htmlPart.hasObject("image.png"));
		VASSERT_EQ("absolute", true, htmlPart.hasObject("http://www.vmime.org/test/image.png"));
		VASSERT_EQ("content-location", false, htmlPart.hasObject("../image.png"));
	}

	/** Test that "cid:" URIs only reference Content-Ids, and objects with
	  * both a Content-Id and a Content-Location (RFC 2557, section 8.3).
	  */
	void testParseEmbeddedObjectsLocationCID() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"cid:image1@vmime.org\"/>\r\n"
"<img src=\"cid:image2@vmime.org\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-ID: <image1@vmime.org>\r\n"
"Content-Location: image1.png\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: CID:image2@vmime.org\r\n"
"\r\n"
"Image2\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		// A single object for the part with both fields, and the
		// "CID:" Content-Location is ignored
		VASSERT_EQ("count", 1, htmlPart.getObjectCount());

		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> obj =
			htmlPart.findObject("cid:image1@vmime.org");

		VASSERT("obj1", obj);
		VASSERT_EQ("ref-type1", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_ID, obj->getReferenceType());
		VASSERT_EQ("id-obj1", "image1@vmime.org", obj->getId());
		VASSERT("obj1-location", htmlPart.findObject("image1.png") == obj);

		VASSERT_EQ("has-obj2", false, htmlPart.hasObject("cid:image2@vmime.org"));
		VASSERT_EQ("has-obj2-location", false, htmlPart.hasObject("CID:image2@vmime.org"));
	}

	/** Test references between nested multipart/related parts
	  * (RFC 2557, section 9.6).
	  */
	void testParseEmbeddedObjectsLocationNested() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"http://www.vmime.org/images/image1.png\"/>\r\n"
"<img src=\"http://www.vmime.org/images/image2.png\"/>\r\n"
"<a href=\"http://www.vmime.org/more-info\">More info</a>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/images/image1.png\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL1\r\n"
"Content-Location: http://www.vmime.org/more-info\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL2a\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL2a\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"images/image1.png\"/>\r\n"
"<img src=\"images/image2.png\"/>\r\n"
"<img src=\"images/image3.png\"/>\r\n"
"--LEVEL2a\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/images/image2.png\r\n"
"\r\n"
"Image2\r\n"
"--LEVEL2a--\r\n"
"--LEVEL1\r\n"
"Content-Location: http://www.vmime.org/even-more-info\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL2b\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL2b\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"images/image3.png\"/>\r\n"
"--LEVEL2b\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/images/image3.png\r\n"
"\r\n"
"Image3\r\n"
"--LEVEL2b--\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		// Nested text: objects of its multipart/related part and of the
		// surrounding one, but not those of a parallel one
		vmime::shared_ptr <vmime::bodyPart> nestedPart = msg->getBody()->getPartAt(2);

		vmime::htmlTextPart nestedHtmlPart;
		nestedHtmlPart.parse(msg, nestedPart, nestedPart->getBody()->getPartAt(0));

		VASSERT_EQ("nested-count", 2, nestedHtmlPart.getObjectCount());
		VASSERT("nested-obj1", nestedHtmlPart.findObject("images/image1.png"));
		VASSERT_EQ("nested-data1", "Image1", extractContent(nestedHtmlPart.findObject("images/image1.png")->getData()));
		VASSERT("nested-obj2", nestedHtmlPart.findObject("images/image2.png"));
		VASSERT_EQ("nested-data2", "Image2", extractContent(nestedHtmlPart.findObject("images/image2.png")->getData()));
		VASSERT_EQ("nested-obj3", false, nestedHtmlPart.hasObject("images/image3.png"));

		// Outer text: objects of nested multipart/related parts cannot
		// be referenced
		vmime::htmlTextPart outerHtmlPart;
		outerHtmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("outer-count", 1, outerHtmlPart.getObjectCount());
		VASSERT_EQ("outer-obj1", true, outerHtmlPart.hasObject("http://www.vmime.org/images/image1.png"));
		VASSERT_EQ("outer-obj2", false, outerHtmlPart.hasObject("http://www.vmime.org/images/image2.png"));
	}

	/** Test an encoded Content-Location (RFC 2557, section 4.4.1).
	  */
	void testParseEmbeddedObjectsLocationEncoded() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"utf-8\"\r\n"
"\r\n"
"<img src=\"http://www.vmime.org/caf\xc3\xa9.png\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: =?utf-8?Q?http://www.vmime.org/caf=C3=A9.png?=\r\n"
"\r\n"
"Image\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("count", 1, htmlPart.getObjectCount());
		VASSERT_EQ("id", "http://www.vmime.org/caf\xc3\xa9.png", htmlPart.getObjectAt(0)->getId());
		VASSERT_EQ("has-obj", true, htmlPart.hasObject("http://www.vmime.org/caf\xc3\xa9.png"));
	}

	/** Test that Content-Location fields are only used in multipart/related
	  * parts (RFC 2557, section 7).
	  */
	void testParseEmbeddedObjectsLocationNotRelated() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/mixed; boundary=\"LEVEL1\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"http://www.vmime.org/image.png\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/image.png\r\n"
"\r\n"
"Image\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("count", 0, htmlPart.getObjectCount());
		VASSERT_EQ("has-obj", false, htmlPart.hasObject("http://www.vmime.org/image.png"));
	}

	static vmime::shared_ptr <vmime::htmlTextPart> newHTMLTextPart(vmime::messageBuilder& mb) {

		mb.getRecipients().appendAddress(vmime::make_shared <vmime::mailbox>("you@vmime.org"));
		mb.constructTextPart(vmime::mediaType(vmime::mediaTypes::TEXT, vmime::mediaTypes::TEXT_HTML));

		vmime::shared_ptr <vmime::htmlTextPart> textPart =
			vmime::dynamicCast <vmime::htmlTextPart>(mb.getTextPart());

		textPart->setCharset(vmime::charset(vmime::charsets::US_ASCII));

		return textPart;
	}

	static vmime::shared_ptr <vmime::message> generateAndParse(const vmime::messageBuilder& mb) {

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(mb.construct()->generate());

		return msg;
	}

	/** Test generation of objects referenced by a Content-Location.
	  */
	void testGenerateEmbeddedObjectsLocation() {

		vmime::messageBuilder mb;
		vmime::shared_ptr <vmime::htmlTextPart> textPart = newHTMLTextPart(mb);

		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> locObj =
			textPart->addObject(
				vmime::make_shared <vmime::stringContentHandler>("Image1"),
				vmime::mediaType("image/png"),
				"images/image1.png"
			);

		VASSERT_EQ("ref-type", vmime::htmlTextPart::embeddedObject::REFERENCED_BY_LOCATION, locObj->getReferenceType());
		VASSERT_EQ("id", "images/image1.png", locObj->getId());
		VASSERT_EQ("ref-id", "images/image1.png", locObj->getReferenceId());
		VASSERT("find", textPart->findObject("images/image1.png") == locObj);

		vmime::shared_ptr <const vmime::htmlTextPart::embeddedObject> cidObj =
			textPart->addObject(
				vmime::make_shared <vmime::stringContentHandler>("Image2"),
				vmime::mediaType("image/png")
			);

		textPart->setText(
			vmime::make_shared <vmime::stringContentHandler>(
				"<img src=\"images/image1.png\"/><img src=\"" + cidObj->getReferenceId() + "\"/>"
			)
		);

		vmime::shared_ptr <vmime::message> msg = generateAndParse(mb);

		// The type of the root part is required (RFC 2387, section 3.1)
		vmime::shared_ptr <const vmime::contentTypeField> ctf =
			msg->getHeader()->findField <vmime::contentTypeField>(vmime::fields::CONTENT_TYPE);

		VASSERT_EQ("type", "multipart/related", ctf->getValue <vmime::mediaType>()->generate());
		VASSERT("type-param", ctf->hasParameter("type"));
		VASSERT_EQ("type-param-value", "text/html", ctf->findParameter("type")->getValue().getBuffer());

		VASSERT_EQ("part-count", 3, msg->getBody()->getPartCount());

		vmime::shared_ptr <const vmime::header> locHdr = msg->getBody()->getPartAt(1)->getHeader();

		VASSERT_EQ("loc-location", "images/image1.png",
			locHdr->findFieldValue <vmime::text>(vmime::fields::CONTENT_LOCATION)->getWholeBuffer());
		VASSERT_EQ("loc-cid", false, locHdr->hasField(vmime::fields::CONTENT_ID));

		vmime::shared_ptr <const vmime::header> cidHdr = msg->getBody()->getPartAt(2)->getHeader();

		VASSERT_EQ("cid-cid", true, cidHdr->hasField(vmime::fields::CONTENT_ID));
		VASSERT_EQ("cid-location", false, cidHdr->hasField(vmime::fields::CONTENT_LOCATION));

		// Objects are found back when the message is parsed
		vmime::messageParser mp(msg);

		VASSERT_EQ("attachments", 0, mp.getAttachmentCount());
		VASSERT_EQ("text-parts", 1, mp.getTextPartCount());

		vmime::shared_ptr <const vmime::htmlTextPart> parsedPart =
			vmime::dynamicCast <const vmime::htmlTextPart>(mp.getTextPartAt(0));

		VASSERT_EQ("count", 2, parsedPart->getObjectCount());
		VASSERT_EQ("data-obj1", "Image1", extractContent(parsedPart->findObject("images/image1.png")->getData()));
		VASSERT_EQ("data-obj2", "Image2", extractContent(parsedPart->findObject(cidObj->getReferenceId())->getData()));
	}

	/** Test that a long Content-Location is folded (RFC 2557, section 4.4.2).
	  */
	void testGenerateEmbeddedObjectsLongLocation() {

		const vmime::string location =
			"http://www.vmime.org/a/very/long/path/to/an/image/which/does/not/fit/on/one/line/image.png";

		vmime::messageBuilder mb;
		vmime::shared_ptr <vmime::htmlTextPart> textPart = newHTMLTextPart(mb);

		textPart->addObject(
			vmime::make_shared <vmime::stringContentHandler>("Image"),
			vmime::mediaType("image/png"),
			location
		);

		textPart->setText(vmime::make_shared <vmime::stringContentHandler>("<img src=\"" + location + "\"/>"));

		const vmime::string generated = mb.construct()->generate();

		// No encoding, and no line longer than 78 characters
		VASSERT_EQ("not-encoded", vmime::string::npos, generated.find("=?"));

		std::istringstream iss(generated);
		vmime::string line;

		while (std::getline(iss, line)) {
			VASSERT("line-length", line.length() <= 79);  // with CR
		}

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(generated);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		VASSERT_EQ("count", 1, htmlPart.getObjectCount());
		VASSERT_EQ("id", location, htmlPart.getObjectAt(0)->getId());
		VASSERT_EQ("has-obj", true, htmlPart.hasObject(location));
	}

	/** Test generation of parsed objects: their fields are kept.
	  */
	void testGenerateParsedEmbeddedObjects() {

		const vmime::string msgString = ""
"MIME-Version: 1.0\r\n"
"Content-Type: multipart/related; boundary=\"LEVEL1\"; type=\"text/html\"\r\n"
"\r\n"
"--LEVEL1\r\n"
"Content-Type: text/html; charset=\"us-ascii\"\r\n"
"\r\n"
"<img src=\"cid:image1@vmime.org\"/>\r\n"
"<img src=\"http://www.vmime.org/image2.png\"/>\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-ID: <image1@vmime.org>\r\n"
"Content-Location: image1.png\r\n"
"\r\n"
"Image1\r\n"
"--LEVEL1\r\n"
"Content-Type: image/png\r\n"
"Content-Location: http://www.vmime.org/image2.png\r\n"
"\r\n"
"Image2\r\n"
"--LEVEL1--\r\n"
"";

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->parse(msgString);

		vmime::htmlTextPart htmlPart;
		htmlPart.parse(msg, msg, msg->getBody()->getPartAt(0));

		vmime::shared_ptr <vmime::message> newMsg = vmime::make_shared <vmime::message>();
		newMsg->getHeader()->ContentType()->setValue(vmime::mediaType("multipart/mixed"));

		htmlPart.generateIn(newMsg, newMsg);

		vmime::shared_ptr <const vmime::bodyPart> relPart = newMsg->getBody()->getPartAt(0);

		VASSERT_EQ("part-count", 3, relPart->getBody()->getPartCount());

		// Both fields of the first object
		vmime::shared_ptr <const vmime::header> hdr1 = relPart->getBody()->getPartAt(1)->getHeader();

		VASSERT_EQ("cid1", "image1@vmime.org",
			hdr1->findFieldValue <vmime::messageId>(vmime::fields::CONTENT_ID)->getId());
		VASSERT_EQ("location1", "image1.png",
			hdr1->findFieldValue <vmime::text>(vmime::fields::CONTENT_LOCATION)->getWholeBuffer());

		// The URI is not written as a Content-Id
		vmime::shared_ptr <const vmime::header> hdr2 = relPart->getBody()->getPartAt(2)->getHeader();

		VASSERT_EQ("cid2", false, hdr2->hasField(vmime::fields::CONTENT_ID));
		VASSERT_EQ("location2", "http://www.vmime.org/image2.png",
			hdr2->findFieldValue <vmime::text>(vmime::fields::CONTENT_LOCATION)->getWholeBuffer());
	}

	void testAddObjectInvalidLocation() {

		vmime::htmlTextPart htmlPart;

		const vmime::shared_ptr <vmime::contentHandler> data =
			vmime::make_shared <vmime::stringContentHandler>("Image");

		const vmime::mediaType type("image/png");

		VASSERT_THROW("empty", htmlPart.addObject(data, type, ""), vmime::exceptions::invalid_argument);
		VASSERT_THROW("space", htmlPart.addObject(data, type, "my image.png"), vmime::exceptions::invalid_argument);
		VASSERT_THROW("crlf", htmlPart.addObject(data, type, "image.png\r\nX-Header: x"), vmime::exceptions::invalid_argument);

		VASSERT_EQ("count", 0, htmlPart.getObjectCount());
	}

	// TODO: test generation of text parts

VMIME_TEST_SUITE_END
