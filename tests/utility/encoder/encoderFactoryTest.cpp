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

#include "vmime/utility/encoder/noopEncoder.hpp"



VMIME_TEST_SUITE_BEGIN(encoderFactoryTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testNoDefaultEncoder)
		VMIME_TEST(testDefaultEncoder)
	VMIME_TEST_LIST_END


	void testNoDefaultEncoder()
	{
		vmime::shared_ptr <vmime::utility::encoder::encoderFactory> ef =
			vmime::utility::encoder::encoderFactory::getInstance();

		VASSERT_THROW(
			"no default encoder",
			ef->create("non-existing-encoding"),
			vmime::exceptions::no_encoder_available
		);
	}

	void testDefaultEncoder()
	{
		vmime::shared_ptr <vmime::utility::encoder::encoderFactory> ef =
			vmime::utility::encoder::encoderFactory::getInstance();

		ef->setDefaultEncoder(vmime::make_shared <vmime::utility::encoder::noopEncoder>());

		VASSERT_NO_THROW(
			"default encoder",
			ef->create("non-existing-encoding")
		);
	}

VMIME_TEST_SUITE_END
