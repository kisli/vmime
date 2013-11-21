//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "vmime/net/imap/IMAPTag.hpp"


VMIME_TEST_SUITE_BEGIN(imapTagTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstruct)
		VMIME_TEST(testIncrement)
		VMIME_TEST(testReset)
		VMIME_TEST(testNumber)
	VMIME_TEST_LIST_END


	void testConstruct()
	{
		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		VASSERT_EQ("init", "a001", static_cast <vmime::string>(*tag));
	}

	void testIncrement()
	{
		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		(*tag)++;
		VASSERT_EQ("init", "a002", static_cast <vmime::string>(*tag));

		(*tag)++;
		VASSERT_EQ("init", "a003", static_cast <vmime::string>(*tag));

		(*tag)++;
		VASSERT_EQ("init", "a004", static_cast <vmime::string>(*tag));
	}

	void testReset()
	{
		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		for (int i = tag->number() ; i < tag->maximumNumber() ; ++i)
			(*tag)++;

		VASSERT_EQ("last", "Z999", static_cast <vmime::string>(*tag));

		(*tag)++;

		VASSERT_EQ("reset", "a001", static_cast <vmime::string>(*tag));
	}

	void testNumber()
	{
		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		for (int i = 0 ; i < 41 ; ++i)
			(*tag)++;

		VASSERT_EQ("number", 42, tag->number());
	}

VMIME_TEST_SUITE_END
