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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "vmime/utility/url.hpp"
#include "vmime/utility/urlUtils.hpp"

using namespace unitpp;


namespace
{
	class urlTest : public suite
	{
		static const bool parseHelper(vmime::utility::url& u, const vmime::string& str)
		{
			try
			{
				u = vmime::utility::url(str);
			}
			catch (vmime::exceptions::malformed_url)
			{
				return false;
			}

			return true;
		}


		void testParse1()
		{
			// Test some valid constructions
			vmime::utility::url u1("", "");

			assert_eq("1.1", true, parseHelper(u1, "protocol://user:password@host:12345/path/"));
			assert_eq("1.2", "protocol", u1.getProtocol());
			assert_eq("1.3", "user", u1.getUsername());
			assert_eq("1.4", "password", u1.getPassword());
			assert_eq("1.5", "host", u1.getHost());
			assert_eq("1.6", 12345, u1.getPort());
			assert_eq("1.7", "/path/", u1.getPath());

			vmime::utility::url u2("", "");

			assert_eq("2.1", true, parseHelper(u2, "protocol://user@host:12345/path/"));
			assert_eq("2.2", "protocol", u2.getProtocol());
			assert_eq("2.3", "user", u2.getUsername());
			assert_eq("2.4", "", u2.getPassword());
			assert_eq("2.5", "host", u2.getHost());
			assert_eq("2.6", 12345, u2.getPort());
			assert_eq("2.7", "/path/", u2.getPath());

			vmime::utility::url u3("", "");

			assert_eq("3.1", true, parseHelper(u3, "protocol://host:12345/path/"));
			assert_eq("3.2", "protocol", u3.getProtocol());
			assert_eq("3.3", "", u3.getUsername());
			assert_eq("3.4", "", u3.getPassword());
			assert_eq("3.5", "host", u3.getHost());
			assert_eq("3.6", 12345, u3.getPort());
			assert_eq("3.7", "/path/", u3.getPath());

			vmime::utility::url u4("", "");

			assert_eq("4.1", true, parseHelper(u4, "protocol://host/path/"));
			assert_eq("4.2", "protocol", u4.getProtocol());
			assert_eq("4.3", "", u4.getUsername());
			assert_eq("4.4", "", u4.getPassword());
			assert_eq("4.5", "host", u4.getHost());
			assert_eq("4.6", vmime::utility::url::UNSPECIFIED_PORT, u4.getPort());
			assert_eq("4.7", "/path/", u4.getPath());

			vmime::utility::url u5("", "");

			assert_eq("5.1", true, parseHelper(u5, "protocol://host/"));
			assert_eq("5.2", "protocol", u5.getProtocol());
			assert_eq("5.3", "", u5.getUsername());
			assert_eq("5.4", "", u5.getPassword());
			assert_eq("5.5", "host", u5.getHost());
			assert_eq("5.6", vmime::utility::url::UNSPECIFIED_PORT, u4.getPort());
			assert_eq("5.7", "", u5.getPath());

			vmime::utility::url u6("", "");

			assert_eq("6.1", true, parseHelper(u4, "protocol://host/path/file"));
			assert_eq("6.2", "protocol", u4.getProtocol());
			assert_eq("6.3", "", u4.getUsername());
			assert_eq("6.4", "", u4.getPassword());
			assert_eq("6.5", "host", u4.getHost());
			assert_eq("6.6", vmime::utility::url::UNSPECIFIED_PORT, u4.getPort());
			assert_eq("6.7", "/path/file", u4.getPath());
		}

		void testParse2()
		{
			// Now, test some ill-formed URLs

			// -- missing protocol
			vmime::utility::url u1("", "");
			assert_eq("1", false, parseHelper(u1, "://host"));

			// -- port can contain only digits
			vmime::utility::url u2("", "");
			assert_eq("2", false, parseHelper(u2, "proto://host:abc123"));

			// -- no host specified
			vmime::utility::url u3("", "");
			assert_eq("3", false, parseHelper(u3, "proto:///path"));

			// -- no protocol separator (://)
			vmime::utility::url u4("", "");
			assert_eq("4", false, parseHelper(u4, "protohost/path"));
		}

		void testParse3()
		{
			// Test decoding
			vmime::utility::url u1("", "");

			assert_eq("1.1", true, parseHelper(u1, "pro%12to://user%34:pass%56word@ho%78st:12345/pa%abth/"));
			assert_eq("1.2", "pro%12to", u1.getProtocol());  // protocol should not be decoded
			assert_eq("1.3", "user\x34", u1.getUsername());
			assert_eq("1.4", "pass\x56word", u1.getPassword());
			assert_eq("1.5", "ho\x78st", u1.getHost());
			assert_eq("1.6", 12345, u1.getPort());
			assert_eq("1.7", "/pa\xabth/", u1.getPath());
		}

		void testGenerate()
		{
			vmime::utility::url u1("proto", "host", 12345, "path", "user", "password");
			assert_eq("1", "proto://user:password@host:12345/path",
				static_cast <vmime::string>(u1));

			// TODO: more tests
		}

		void testUtilsEncode()
		{
			assert_eq("1", "%01", vmime::utility::urlUtils::encode("\x01"));
			assert_eq("2", "%20", vmime::utility::urlUtils::encode(" "));
			assert_eq("3", "%FF", vmime::utility::urlUtils::encode("\xff"));
			assert_eq("4", "a", vmime::utility::urlUtils::encode("a"));
		}

		void testUtilsDecode()
		{
			for (int i = 0 ; i < 255 ; ++i)
			{
				std::ostringstream ossTest;
				ossTest << "%" << "0123456789ABCDEF"[i / 16]
				               << "0123456789ABCDEF"[i % 16];

				std::ostringstream ossNum;
				ossNum << i;

				vmime::string res;
				res += static_cast <unsigned char>(i);

				assert_eq(ossNum.str(), res,
					vmime::utility::urlUtils::decode(ossTest.str()));
			}

		}

	public:

		urlTest() : suite("vmime::utility::url")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse1", testcase(this, "Parse1", &urlTest::testParse1));
			add("Parse2", testcase(this, "Parse2", &urlTest::testParse2));
			add("Parse3", testcase(this, "Parse3", &urlTest::testParse3));
			add("Generate", testcase(this, "Generate", &urlTest::testGenerate));
			add("UtilsEncode", testcase(this, "UtilsEncode", &urlTest::testUtilsEncode));
			add("UtilsDecode", testcase(this, "UtilsDecode", &urlTest::testUtilsDecode));

			suite::main().add("vmime::utility::url", this);
		}

	};

	urlTest* theTest = new urlTest();
}
