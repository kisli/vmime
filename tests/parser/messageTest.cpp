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


VMIME_TEST_SUITE_BEGIN(messageTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGetGeneratedSize)
	VMIME_TEST_LIST_END


	void testGetGeneratedSize()
	{
		vmime::generationContext ctx;

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();
		msg->getHeader()->getField("Foo")->setValue(vmime::string("bar"));

		vmime::htmlTextPart textPart;
		textPart.setPlainText(vmime::make_shared <vmime::stringContentHandler>("Foo bar bazé foo foo foo"));
		textPart.setText(vmime::make_shared <vmime::stringContentHandler>("Foo bar <strong>bazé</strong> foo foo foo"));
		textPart.generateIn(msg, msg);

		// Estimated/computed generated size must be greater than the actual generated size
		const vmime::size_t genSize = msg->getGeneratedSize(ctx);
		const vmime::size_t actualSize = msg->generate().length();

		std::ostringstream oss;
		oss << "estimated size (" << genSize << ") >= actual size (" << actualSize << ")";

		VASSERT(oss.str(), genSize >= actualSize);
	}

VMIME_TEST_SUITE_END

