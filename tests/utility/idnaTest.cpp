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

#include "vmime/utility/idna.hpp"


VMIME_TEST_SUITE_BEGIN(idnaTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testToASCII)
		VMIME_TEST(testToASCII_ASCII)
		VMIME_TEST(testToASCII_Mapping)
		VMIME_TEST(testToASCII_Invalid)
		VMIME_TEST(testToUnicode)
		VMIME_TEST(testToUnicode_InvalidALabel)
	VMIME_TEST_LIST_END


	static vmime::string toASCII(const vmime::string& domain) {

		vmime::string out;

		if (!vmime::utility::idna::toASCII(domain, out)) {
			return "(invalid)";
		}

		return out;
	}

	void testToASCII() {

		if (!vmime::utility::idna::isSupported()) {

			VASSERT_EQ("No IDNA support", "(invalid)", toASCII("bücher.example"));
			return;
		}

		VASSERT_EQ("1", "xn--bcher-kva.example", toASCII("bücher.example"));

		// Tests from ICANN
		VASSERT_EQ("2.1", "xn--hxajbheg2az3al.xn--jxalpdlp", toASCII("παράδειγμα.δοκιμή"));
		VASSERT_EQ("2.2", "xn--mgbh0fb.xn--kgbechtv", toASCII("مثال.إختبار"));
		VASSERT_EQ("2.3", "xn--r8jz45g.xn--zckzah", toASCII("例え.テスト"));

		// No transitional processing: "ß" is not mapped to "ss" (UTS #46, section 4)
		VASSERT_EQ("3", "xn--fa-hia.example", toASCII("faß.example"));
	}

	// ASCII domain names are not checked, nor modified
	void testToASCII_ASCII() {

		VASSERT_EQ("1", "Example.COM", toASCII("Example.COM"));
		VASSERT_EQ("2", "xn--paypal-.example", toASCII("xn--paypal-.example"));
		VASSERT_EQ("3", "[192.0.2.1]", toASCII("[192.0.2.1]"));
	}

	// User input is mapped before it is converted (UTS #46, section 4)
	void testToASCII_Mapping() {

		if (!vmime::utility::idna::isSupported()) {
			return;
		}

		VASSERT_EQ("Uppercase", "xn--bcher-kva.example", toASCII("BÜCHER.EXAMPLE"));
		VASSERT_EQ("NFD", "xn--bcher-kva.example", toASCII("bu\xcc\x88" "cher.example"));
		VASSERT_EQ("Fullwidth", "xn--bcher-kva.example", toASCII("ｂüｃｈｅｒ.example"));
		VASSERT_EQ("Ideographic full stop", "xn--r8jz45g.xn--zckzah", toASCII("例え。テスト"));
	}

	// Labels must follow the rules of IDNA2008 (RFC 5891, section 5.4)
	void testToASCII_Invalid() {

		// Disallowed character (U+2024 ONE DOT LEADER)
		VASSERT_EQ("Disallowed", "(invalid)", toASCII("paypal\xe2\x80\xa4" "com.example"));

		// Leading combining mark
		VASSERT_EQ("Combining mark", "(invalid)", toASCII("\xcc\x88" "a.example"));

		// A label which starts with a right-to-left character must not
		// contain left-to-right characters (RFC 5893, section 2)
		VASSERT_EQ("Bidi", "(invalid)", toASCII("\xd7\x90" "1a.example"));

		// Labels are at most 63 octets long, also as A-labels
		vmime::string longLabel;

		for (int i = 0 ; i < 64 ; ++i) {
			longLabel += "ü";
		}

		VASSERT_EQ("Too long", "(invalid)", toASCII(longLabel + ".example"));

		// Labels are host names (RFC 5321, section 4.1.2): other characters
		// are not allowed, and they are not removed either
		VASSERT_EQ("Host name 1", "(invalid)", toASCII("bücher.my_host.example"));
		VASSERT_EQ("Host name 2", "(invalid)", toASCII("bü_cher.example"));
		VASSERT_EQ("Host name 3", "(invalid)", toASCII("bü@cher.example"));
		VASSERT_EQ("Host name 4", "(invalid)", toASCII("bü cher.example"));

		// Invalid UTF-8
		VASSERT_EQ("UTF-8", "(invalid)", toASCII("b\xc3\xbc" "\xff" "cher.example"));
		VASSERT_EQ("Truncated UTF-8", "(invalid)", toASCII("b\xc3\xbc" "cher\xe2"));
		VASSERT_EQ("NUL", "(invalid)", toASCII(vmime::string("b\xc3\xbc" "cher") + '\0' + ".example"));
	}

	void testToUnicode() {

		if (!vmime::utility::idna::isSupported()) {

			VASSERT_EQ("No IDNA support", "xn--bcher-kva.example", vmime::utility::idna::toUnicode("xn--bcher-kva.example"));
			return;
		}

		VASSERT_EQ("1", "bücher.example", vmime::utility::idna::toUnicode("xn--bcher-kva.example"));
		VASSERT_EQ("2", "黒川.日本", vmime::utility::idna::toUnicode("xn--5rtw95l.xn--wgv71a"));

		// The ACE prefix is case-insensitive, and A-labels are converted to
		// lowercase before they are decoded (RFC 5891, section 5.3)
		VASSERT_EQ("3", "bücher.example", vmime::utility::idna::toUnicode("XN--BCHER-KVA.example"));

		// Other labels are not modified
		VASSERT_EQ("4", "www.bücher.Example.", vmime::utility::idna::toUnicode("www.xn--bcher-kva.Example."));
		VASSERT_EQ("5", "bücher.example", vmime::utility::idna::toUnicode("bücher.example"));
		VASSERT_EQ("6", "", vmime::utility::idna::toUnicode(""));
	}

	// Labels which look like A-labels, but are not, are not converted
	// (RFC 5891, section 5.3)
	void testToUnicode_InvalidALabel() {

		// Invalid Punycode
		VASSERT_EQ("1", "xn--zzzzzzzzz.example", vmime::utility::idna::toUnicode("xn--zzzzzzzzz.example"));

		// "paypal": an A-label never decodes to an ASCII string
		VASSERT_EQ("2", "xn--paypal-.example", vmime::utility::idna::toUnicode("xn--paypal-.example"));

		// "paypal․com", with U+2024 ONE DOT LEADER, which is disallowed
		VASSERT_EQ("3", "xn--paypalcom-qe0e.example", vmime::utility::idna::toUnicode("xn--paypalcom-qe0e.example"));

		// "BÜCHER": uppercase letters are disallowed in U-labels
		VASSERT_EQ("4", "xn--BCHER-2pa.example", vmime::utility::idna::toUnicode("xn--BCHER-2pa.example"));

		// "a​b": ZERO WIDTH SPACE is disallowed
		VASSERT_EQ("5", "xn--ab-g1t.example", vmime::utility::idna::toUnicode("xn--ab-g1t.example"));

		// Only valid A-labels are converted
		if (vmime::utility::idna::isSupported()) {

			VASSERT_EQ(
				"6", "xn--zzzzzzzzz.bücher.example",
				vmime::utility::idna::toUnicode("xn--zzzzzzzzz.xn--bcher-kva.example")
			);
		}
	}

VMIME_TEST_SUITE_END
