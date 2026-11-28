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

#include "vmime/utility/uriReference.hpp"


VMIME_TEST_SUITE_BEGIN(uriReferenceTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testParseEmptyComponents)
		VMIME_TEST(testParseRelative)
		VMIME_TEST(testToString)
		VMIME_TEST(testResolve)
		VMIME_TEST(testResolveAbnormal)
		VMIME_TEST(testResolveNoAuthority)
	VMIME_TEST_LIST_END


	void testParse() {

		const vmime::utility::uriReference ref("http://user@www.vmime.org:80/a/b?x=1&y=2#top");

		VASSERT_TRUE("scheme", ref.hasScheme());
		VASSERT_EQ("scheme-value", "http", ref.getScheme());
		VASSERT_TRUE("authority", ref.hasAuthority());
		VASSERT_EQ("authority-value", "user@www.vmime.org:80", ref.getAuthority());
		VASSERT_EQ("path", "/a/b", ref.getPath());
		VASSERT_TRUE("query", ref.hasQuery());
		VASSERT_EQ("query-value", "x=1&y=2", ref.getQuery());
		VASSERT_TRUE("fragment", ref.hasFragment());
		VASSERT_EQ("fragment-value", "top", ref.getFragment());
	}

	void testParseEmptyComponents() {

		// Empty components are not undefined components
		const vmime::utility::uriReference ref1("file://?#");

		VASSERT_TRUE("1.authority", ref1.hasAuthority());
		VASSERT_EQ("1.authority-value", "", ref1.getAuthority());
		VASSERT_EQ("1.path", "", ref1.getPath());
		VASSERT_TRUE("1.query", ref1.hasQuery());
		VASSERT_EQ("1.query-value", "", ref1.getQuery());
		VASSERT_TRUE("1.fragment", ref1.hasFragment());
		VASSERT_EQ("1.fragment-value", "", ref1.getFragment());

		const vmime::utility::uriReference ref2("cid:image@vmime.org");

		VASSERT_TRUE("2.scheme", ref2.hasScheme());
		VASSERT_EQ("2.scheme-value", "cid", ref2.getScheme());
		VASSERT_FALSE("2.authority", ref2.hasAuthority());
		VASSERT_EQ("2.path", "image@vmime.org", ref2.getPath());
		VASSERT_FALSE("2.query", ref2.hasQuery());
		VASSERT_FALSE("2.fragment", ref2.hasFragment());
	}

	void testParseRelative() {

		const vmime::utility::uriReference ref1("images/logo.gif");

		VASSERT_FALSE("1.scheme", ref1.hasScheme());
		VASSERT_FALSE("1.authority", ref1.hasAuthority());
		VASSERT_EQ("1.path", "images/logo.gif", ref1.getPath());

		const vmime::utility::uriReference ref2("//www.vmime.org/logo.gif");

		VASSERT_FALSE("2.scheme", ref2.hasScheme());
		VASSERT_TRUE("2.authority", ref2.hasAuthority());
		VASSERT_EQ("2.authority-value", "www.vmime.org", ref2.getAuthority());
		VASSERT_EQ("2.path", "/logo.gif", ref2.getPath());

		// Not a scheme: it does not start with a letter
		const vmime::utility::uriReference ref3("1a:b");

		VASSERT_FALSE("3.scheme", ref3.hasScheme());
		VASSERT_EQ("3.path", "1a:b", ref3.getPath());

		// Not a scheme: it contains a character which is not allowed
		const vmime::utility::uriReference ref4("a_b:c");

		VASSERT_FALSE("4.scheme", ref4.hasScheme());
		VASSERT_EQ("4.path", "a_b:c", ref4.getPath());
	}

	void testToString() {

		VASSERT_EQ("1", "http://www.vmime.org/a/b?x=1#top",
			vmime::utility::uriReference("http://www.vmime.org/a/b?x=1#top").toString());
		VASSERT_EQ("2", "file://?#", vmime::utility::uriReference("file://?#").toString());
		VASSERT_EQ("3", "", vmime::utility::uriReference("").toString());

		// Percent-encoded octets are not decoded (RFC 2557, section 8.2)
		VASSERT_EQ("4", "a%2eb/c%20d", vmime::utility::uriReference("a%2eb/c%20d").toString());
	}

	static vmime::string resolve(const vmime::string& ref) {

		return vmime::utility::uriReference(ref).resolve(
			vmime::utility::uriReference("http://a/b/c/d;p?q")
		).toString();
	}

	// RFC 3986, section 5.4.1
	void testResolve() {

		VASSERT_EQ("1",  "g:h", resolve("g:h"));
		VASSERT_EQ("2",  "http://a/b/c/g", resolve("g"));
		VASSERT_EQ("3",  "http://a/b/c/g", resolve("./g"));
		VASSERT_EQ("4",  "http://a/b/c/g/", resolve("g/"));
		VASSERT_EQ("5",  "http://a/g", resolve("/g"));
		VASSERT_EQ("6",  "http://g", resolve("//g"));
		VASSERT_EQ("7",  "http://a/b/c/d;p?y", resolve("?y"));
		VASSERT_EQ("8",  "http://a/b/c/g?y", resolve("g?y"));
		VASSERT_EQ("9",  "http://a/b/c/d;p?q#s", resolve("#s"));
		VASSERT_EQ("10", "http://a/b/c/g#s", resolve("g#s"));
		VASSERT_EQ("11", "http://a/b/c/g?y#s", resolve("g?y#s"));
		VASSERT_EQ("12", "http://a/b/c/;x", resolve(";x"));
		VASSERT_EQ("13", "http://a/b/c/g;x", resolve("g;x"));
		VASSERT_EQ("14", "http://a/b/c/g;x?y#s", resolve("g;x?y#s"));
		VASSERT_EQ("15", "http://a/b/c/d;p?q", resolve(""));
		VASSERT_EQ("16", "http://a/b/c/", resolve("."));
		VASSERT_EQ("17", "http://a/b/c/", resolve("./"));
		VASSERT_EQ("18", "http://a/b/", resolve(".."));
		VASSERT_EQ("19", "http://a/b/", resolve("../"));
		VASSERT_EQ("20", "http://a/b/g", resolve("../g"));
		VASSERT_EQ("21", "http://a/", resolve("../.."));
		VASSERT_EQ("22", "http://a/", resolve("../../"));
		VASSERT_EQ("23", "http://a/g", resolve("../../g"));
	}

	// RFC 3986, section 5.4.2
	void testResolveAbnormal() {

		VASSERT_EQ("1",  "http://a/g", resolve("../../../g"));
		VASSERT_EQ("2",  "http://a/g", resolve("../../../../g"));
		VASSERT_EQ("3",  "http://a/g", resolve("/./g"));
		VASSERT_EQ("4",  "http://a/g", resolve("/../g"));
		VASSERT_EQ("5",  "http://a/b/c/g.", resolve("g."));
		VASSERT_EQ("6",  "http://a/b/c/.g", resolve(".g"));
		VASSERT_EQ("7",  "http://a/b/c/g..", resolve("g.."));
		VASSERT_EQ("8",  "http://a/b/c/..g", resolve("..g"));
		VASSERT_EQ("9",  "http://a/b/g", resolve("./../g"));
		VASSERT_EQ("10", "http://a/b/c/g/", resolve("./g/."));
		VASSERT_EQ("11", "http://a/b/c/g/h", resolve("g/./h"));
		VASSERT_EQ("12", "http://a/b/c/h", resolve("g/../h"));
		VASSERT_EQ("13", "http://a/b/c/g;x=1/y", resolve("g;x=1/./y"));
		VASSERT_EQ("14", "http://a/b/c/y", resolve("g;x=1/../y"));
		VASSERT_EQ("15", "http://a/b/c/g?y/./x", resolve("g?y/./x"));
		VASSERT_EQ("16", "http://a/b/c/g?y/../x", resolve("g?y/../x"));
		VASSERT_EQ("17", "http://a/b/c/g#s/./x", resolve("g#s/./x"));
		VASSERT_EQ("18", "http://a/b/c/g#s/../x", resolve("g#s/../x"));
		VASSERT_EQ("19", "http:g", resolve("http:g"));
	}

	// "thismessage:/" is the default base of RFC 2557, section 5
	void testResolveNoAuthority() {

		const vmime::utility::uriReference base("thismessage:/");

		VASSERT_EQ("1", "thismessage:/logo.gif",
			vmime::utility::uriReference("logo.gif").resolve(base).toString());
		VASSERT_EQ("2", "thismessage:/images/logo.gif",
			vmime::utility::uriReference("./images/logo.gif").resolve(base).toString());
		VASSERT_EQ("3", "thismessage:/logo.gif",
			vmime::utility::uriReference("../logo.gif").resolve(base).toString());
		VASSERT_EQ("4", "http://www.vmime.org/logo.gif",
			vmime::utility::uriReference("http://www.vmime.org/logo.gif").resolve(base).toString());
		VASSERT_EQ("5", "thismessage:/a%2eb/c%20d",
			vmime::utility::uriReference("a%2eb/c%20d").resolve(base).toString());
	}

VMIME_TEST_SUITE_END
