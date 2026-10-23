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

#include "encoderTestUtils.hpp"


VMIME_TEST_SUITE_BEGIN(uuEncoderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testEncodeDecode)
		VMIME_TEST(testDecodeBeginMarker)
		VMIME_TEST(testDecodeBeginMarkerWithoutModeAndFilename)
		VMIME_TEST(testDecodeInvalidBeginMarker)
		VMIME_TEST(testDecodeInvalidEndMarker)
		VMIME_TEST(testDecodeOversizedLineLength)
		VMIME_TEST(testDecodeTruncatedMarkers)
	VMIME_TEST_LIST_END


	void testEncodeDecode() {

		// Include lengths which are not a multiple of 3, so that the last group
		// on a line is incomplete, and lengths which span several lines
		static const unsigned int lengths[] = { 0, 1, 2, 3, 4, 44, 45, 46, 47, 100, 256 };

		for (unsigned int i = 0 ; i < sizeof(lengths) / sizeof(lengths[0]) ; ++i) {

			vmime::string data;

			for (unsigned int j = 0 ; j < lengths[i] ; ++j) {
				data += static_cast <char>(j & 0xff);
			}

			std::ostringstream oss;
			oss << "length " << lengths[i];

			VASSERT_EQ(oss.str(), data, decode("uuencode", encode("uuencode", data)));
		}
	}

	void testDecodeBeginMarker() {

		// A well-formed prelude is skipped, and its mode and filename are
		// exposed in the encoder results
		vmime::shared_ptr <vmime::utility::encoder::encoder> enc = getEncoder("uuencode");

		vmime::utility::inputStreamStringAdapter in("begin 644 test.txt\r\nend\r\n");

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter out(oss);

		enc->decode(in, out);

		const vmime::utility::encoder::encoder& cenc = *enc;

		VASSERT_EQ("data", "", oss.str());
		VASSERT_EQ("mode", "644", cenc.getResults().getProperty <vmime::string>("mode", ""));
		VASSERT_EQ("filename", "test.txt", cenc.getResults().getProperty <vmime::string>("filename", ""));
	}

	void testDecodeBeginMarkerWithoutModeAndFilename() {

		// With LF line endings, the space following "begin" is the end of the
		// line itself: the next line holds data, not the mode and filename
		vmime::shared_ptr <vmime::utility::encoder::encoder> enc = getEncoder("uuencode");

		vmime::utility::inputStreamStringAdapter in("begin\n#86)C\nend\n");

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter out(oss);

		enc->decode(in, out);

		const vmime::utility::encoder::encoder& cenc = *enc;

		VASSERT_EQ("data", "abc", oss.str());
		VASSERT_EQ("mode", "644", cenc.getResults().getProperty <vmime::string>("mode", ""));
		VASSERT_EQ("filename", "untitled", cenc.getResults().getProperty <vmime::string>("filename", ""));
	}

	/** A 'b' line length which does not introduce a "begin" line must not make
	  * the per-line read count underflow.
	  *
	  * 'b' decodes to a line length of 2, so the line holds 4 encoded characters,
	  * but the "begin" probe reads 5 bytes before giving up. 'inLength - inPos'
	  * then used to wrap around as an unsigned value, and the resulting
	  * near-SIZE_MAX read count drove a write past the decoder's 64-byte stack
	  * buffer (CWE-191/CWE-787).
	  */
	void testDecodeInvalidBeginMarker() {

		vmime::string data = "b";
		data += "ZZZZZ";          // not "egin ", so the probe leaves inPos == 5
		data.append(512, 'A');     // bytes the oversized read would have run over

		VASSERT_EQ("decoded", "", decode("uuencode", data));
	}

	/** Same shape, on the "end" marker. 'e' decodes to a line length of 5, ie.
	  * 8 encoded characters, and the probe only reads 3 of them, so this path
	  * does not underflow, but it must stay consistent.
	  */
	void testDecodeInvalidEndMarker() {

		// 3 characters are consumed by the probe, then the 5 remaining ones
		const vmime::string data = "eZZZAAAAA\r\n";

		VASSERT_EQ("decoded size", 5, decode("uuencode", data).length());
	}

	/** A line length larger than what the decoder's buffers can hold must not
	  * make it write out bytes it never decoded.
	  *
	  * '_' decodes to a line length of 63, but the decoding loop fills at most
	  * 48 bytes of the output buffer, so 15 bytes of uninitialized stack used
	  * to be copied into the decoded output (CWE-908).
	  */
	void testDecodeOversizedLineLength() {

		vmime::string data = "_";
		data.append(64, 'A');

		// 16 groups of 4 encoded bytes, each decoding to 3 bytes
		vmime::string expected;

		for (int i = 0 ; i < 16 ; ++i) {
			expected += "\x86\x18\x61";
		}

		VASSERT_EQ("decoded", expected, decode("uuencode", data));
	}

	void testDecodeTruncatedMarkers() {

		VASSERT_EQ("b", "", decode("uuencode", "b"));
		VASSERT_EQ("e", "", decode("uuencode", "e"));
		VASSERT_EQ("begin", "", decode("uuencode", "begin 644 test.txt\r\n"));
	}

VMIME_TEST_SUITE_END
