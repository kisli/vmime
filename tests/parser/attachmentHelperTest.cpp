//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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
		// TODO: add more tests
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
	}

VMIME_TEST_SUITE_END

