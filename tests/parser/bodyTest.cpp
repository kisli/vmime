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


VMIME_TEST_SUITE_BEGIN(bodyTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGenerate_Text)
		VMIME_TEST(testGenerate_NonText)
		VMIME_TEST(testGenerate_CanonicalLineEndings)
		VMIME_TEST(testGetGeneratedSize_CanonicalLineEndings)
		VMIME_TEST(testGenerate_Binary)
		VMIME_TEST(testGenerate_ParsedWithLF)
	VMIME_TEST_LIST_END

	void testGenerate_Text() {

		// RFC-2015: [Quoted-Printable encoding] A line break in a text body,
		// represented as a CRLF sequence in the text canonical form, must be
		// represented by a line break which is also a CRLF sequence, in the
		// Quoted-Printable encoding

		vmime::bodyPart p;
		p.getBody()->setContents(
			vmime::make_shared <vmime::stringContentHandler>(
				"Foo éé\r\né bar\r\nbaz"
			),
			vmime::mediaType("text", "plain"),
			vmime::charset("utf-8"),
			vmime::encoding("quoted-printable")
		);

		VASSERT_EQ(
			"generate",
			"Foo =C3=A9=C3=A9\r\n"
			"=C3=A9 bar\r\n"
			"baz",
			p.getBody()->generate()
		);
	}

	void testGenerate_NonText() {

		vmime::bodyPart p;
		p.getBody()->setContents(
			vmime::make_shared <vmime::stringContentHandler>(
				"Binary\xfa\xfb\r\ndata\r\n\r\n\xfc"
			),
			vmime::mediaType("application", "octet-stream"),
			vmime::charset("utf-8"),
			vmime::encoding("quoted-printable")
		);

		VASSERT_EQ(
			"generate",
			"Binary=FA=FB=0D=0Adata=0D=0A=0D=0A=FC",
			p.getBody()->generate()
		);
	}

	static vmime::shared_ptr <vmime::bodyPart> createBodyPart(
		const vmime::string& contents,
		const vmime::mediaType& type,
		const vmime::encoding& enc,
		const vmime::encoding& contentsEnc = vmime::contentHandler::NO_ENCODING
	) {

		vmime::shared_ptr <vmime::bodyPart> p = vmime::make_shared <vmime::bodyPart>();
		p->getBody()->setContents(
			vmime::make_shared <vmime::stringContentHandler>(contents, contentsEnc),
			type, vmime::charset("utf-8"), enc
		);

		return p;
	}

	static vmime::string generateBody(
		const vmime::string& contents,
		const vmime::mediaType& type,
		const vmime::encoding& enc,
		const vmime::encoding& contentsEnc = vmime::contentHandler::NO_ENCODING
	) {

		return createBodyPart(contents, type, enc, contentsEnc)->getBody()->generate();
	}

	// Lines end with CRLF, whatever the convention used by the contents
	// (RFC 5322, section 2.3, and RFC 2045, sections 2.7 and 2.8)
	void testGenerate_CanonicalLineEndings() {

		const vmime::mediaType textPlain("text", "plain");

		VASSERT_EQ(
			"7bit",
			"Line 1\r\nLine 2\r\nLine 3\r\nLine 4\r\n",
			generateBody("Line 1\nLine 2\rLine 3\r\nLine 4\n", textPlain, vmime::encoding("7bit"))
		);

		VASSERT_EQ(
			"8bit",
			"Gr\xc3\xbc\xc3\x9f" "e 1\r\nGr\xc3\xbc\xc3\x9f" "e 2\r\n",
			generateBody("Gr\xc3\xbc\xc3\x9f" "e 1\nGr\xc3\xbc\xc3\x9f" "e 2\n", textPlain, vmime::encoding("8bit"))
		);

		VASSERT_EQ(
			"quoted-printable",
			"Foo =C3=A9=C3=A9\r\n=C3=A9 bar\r\nbaz",
			generateBody("Foo \xc3\xa9\xc3\xa9\n\xc3\xa9 bar\nbaz", textPlain, vmime::encoding("quoted-printable"))
		);

		// Encoded contents, as parsed from a message stored with LF only
		VASSERT_EQ(
			"base64",
			"QUJD\r\nREVG\r\n",
			generateBody(
				"QUJD\nREVG\n", vmime::mediaType("application", "octet-stream"),
				vmime::encoding("base64"), vmime::encoding("base64")
			)
		);
	}

	// The size of contents copied as is takes the conversion of their line
	// endings into account, so that the declared size of a message is not
	// lower than its actual size (RFC 1870, section 6)
	void testGetGeneratedSize_CanonicalLineEndings() {

		const vmime::generationContext& ctx = vmime::generationContext::getDefaultContext();
		const vmime::mediaType textPlain("text", "plain");
		const vmime::mediaType octetStream("application", "octet-stream");

		vmime::shared_ptr <vmime::bodyPart> p1 =
			createBodyPart("Line 1\nLine 2\rLine 3\r\n", textPlain, vmime::encoding("7bit"));

		VASSERT_EQ("7bit", p1->getBody()->generate().length(), p1->getBody()->getGeneratedSize(ctx));

		vmime::shared_ptr <vmime::bodyPart> p2 =
			createBodyPart("Gr\xc3\xbc\xc3\x9f" "e\n", textPlain, vmime::encoding("8bit"));

		VASSERT_EQ("8bit", p2->getBody()->generate().length(), p2->getBody()->getGeneratedSize(ctx));

		vmime::shared_ptr <vmime::bodyPart> p3 = createBodyPart(
			"QUJD\nREVG\n", octetStream, vmime::encoding("base64"), vmime::encoding("base64")
		);

		VASSERT_EQ("base64", p3->getBody()->generate().length(), p3->getBody()->getGeneratedSize(ctx));

		vmime::shared_ptr <vmime::bodyPart> p4 =
			createBodyPart("Binary\ndata\r", octetStream, vmime::encoding("binary"));

		VASSERT_EQ("binary", p4->getBody()->generate().length(), p4->getBody()->getGeneratedSize(ctx));
	}

	// Any sequence of octets is allowed in binary data (RFC 2045, section 2.9)
	void testGenerate_Binary() {

		VASSERT_EQ(
			"binary",
			"Binary\ndata\r",
			generateBody("Binary\ndata\r", vmime::mediaType("application", "octet-stream"), vmime::encoding("binary"))
		);
	}

	// A part parsed from a message stored with LF only (eg. in a Maildir)
	void testGenerate_ParsedWithLF() {

		vmime::bodyPart p;
		p.parse(
			"Content-Type: multipart/mixed; boundary=\"b\"\n"
			"\n"
			"--b\n"
			"Content-Type: text/plain\n"
			"\n"
			"Line 1\nLine 2\n"
			"--b\n"
			"Content-Type: application/octet-stream\n"
			"Content-Transfer-Encoding: base64\n"
			"\n"
			"QUJD\nREVG\n"
			"--b--\n"
		);

		const vmime::string generated = p.generate();

		for (size_t i = 0 ; i < generated.length() ; ++i) {

			if (generated[i] == '\n') {
				VASSERT("CRLF", i > 0 && generated[i - 1] == '\r');
			} else if (generated[i] == '\r') {
				VASSERT("CRLF", i + 1 < generated.length() && generated[i + 1] == '\n');
			}
		}

		VASSERT("Text", generated.find("\r\n\r\nLine 1\r\nLine 2\r\n--b") != vmime::string::npos);
		VASSERT("Base64", generated.find("\r\n\r\nQUJD\r\nREVG\r\n--b") != vmime::string::npos);
	}

VMIME_TEST_SUITE_END
