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

#include "tests/net/pop3/POP3TestUtils.hpp"

#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3SStore.hpp"


VMIME_TEST_SUITE_BEGIN(POP3StoreTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateFromURL)
		VMIME_TEST(testConnectToInvalidServer)
	VMIME_TEST_LIST_END


	void testCreateFromURL()
	{
		vmime::shared_ptr <vmime::net::session> sess
			= vmime::make_shared <vmime::net::session>();

		// POP3
		vmime::utility::url url("pop3://pop3.vmime.org");
		vmime::shared_ptr <vmime::net::store> store = sess->getStore(url);

		VASSERT_TRUE("pop3", typeid(*store) == typeid(vmime::net::pop3::POP3Store));

		// POP3S
		vmime::utility::url url2("pop3s://pop3s.vmime.org");
		vmime::shared_ptr <vmime::net::store> store2 = sess->getStore(url2);

		VASSERT_TRUE("pop3s", typeid(*store2) == typeid(vmime::net::pop3::POP3SStore));
	}

	void testConnectToInvalidServer()
	{
		vmime::shared_ptr <vmime::net::session> sess
			= vmime::make_shared <vmime::net::session>();

		vmime::utility::url url("pop3://invalid-pop3-server");
		vmime::shared_ptr <vmime::net::store> store = sess->getStore(url);

		VASSERT_THROW("connect", store->connect(), vmime::exceptions::connection_error);
	}

VMIME_TEST_SUITE_END
