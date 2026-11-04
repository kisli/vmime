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


VMIME_TEST_SUITE_BEGIN(tracerTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testTraceSendBytes)
		VMIME_TEST(testTraceReceiveBytes)
	VMIME_TEST_LIST_END


	void testTraceSendBytes() {

		// Byte count must be formatted without digit grouping, whatever the global locale
		digitGroupingLocale loc;

		recordingTracer tr;
		tr.traceSendBytes(12288);
		tr.traceSendBytes(12288, "SASL exchange");

		VASSERT_EQ("Count", 2, tr.getSentLines().size());
		VASSERT_EQ("1", "{...12288 bytes of data...}", tr.getSentLines()[0]);
		VASSERT_EQ("2", "{...SASL exchange: 12288 bytes of data...}", tr.getSentLines()[1]);
	}

	void testTraceReceiveBytes() {

		// Byte count must be formatted without digit grouping, whatever the global locale
		digitGroupingLocale loc;

		recordingTracer tr;
		tr.traceReceiveBytes(12288);
		tr.traceReceiveBytes(12288, "SASL exchange");

		VASSERT_EQ("Count", 2, tr.getReceivedLines().size());
		VASSERT_EQ("1", "{...12288 bytes of data...}", tr.getReceivedLines()[0]);
		VASSERT_EQ("2", "{...SASL exchange: 12288 bytes of data...}", tr.getReceivedLines()[1]);
	}

VMIME_TEST_SUITE_END
