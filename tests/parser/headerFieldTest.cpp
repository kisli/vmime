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


VMIME_TEST_SUITE_BEGIN(headerFieldTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testBadValueType)
		VMIME_TEST(testValueOnNextLine)
		VMIME_TEST(testStripSpacesAtEnd)
		VMIME_TEST(testValueWithEmptyLine)
	VMIME_TEST_LIST_END


	void testBadValueType()
	{
		vmime::shared_ptr <vmime::headerFieldFactory> hfactory =
			vmime::headerFieldFactory::getInstance();

		// "To" header field accepts values of type "addressList"
		vmime::shared_ptr <vmime::headerField> to = hfactory->create(vmime::fields::TO);
		VASSERT_THROW("to",
			to->setValue(vmime::mailbox("email@vmime.org")),
			vmime::exceptions::bad_field_value_type);

		// Unregistered header field accepts any value type
		vmime::shared_ptr <vmime::headerField> custom = hfactory->create("X-MyCustomHeader");
		VASSERT_NO_THROW("custom/1",
			custom->setValue(vmime::mailbox("email@vmime.org")));
		VASSERT_NO_THROW("custom/2",
			custom->setValue(vmime::text("field value text")));
	}

	void testValueOnNextLine()
	{
		vmime::parsingContext ctx;

		const vmime::string buffer = "Field: \r\n\tfield data";

		vmime::shared_ptr <vmime::headerField> hfield =
			vmime::headerField::parseNext(ctx, buffer, 0, buffer.size());

		vmime::shared_ptr <vmime::text> hvalue =
			hfield->getValue <vmime::text>();

		VASSERT_EQ("Field name", "Field", hfield->getName());
		VASSERT_EQ("Field value", "field data", hvalue->getWholeBuffer());
	}

	void testStripSpacesAtEnd()
	{
		vmime::parsingContext ctx;

		const vmime::string buffer = "Field: \r\n\tfield data   ";

		vmime::shared_ptr <vmime::headerField> hfield =
			vmime::headerField::parseNext(ctx, buffer, 0, buffer.size());

		vmime::shared_ptr <vmime::text> hvalue =
			hfield->getValue <vmime::text>();

		VASSERT_EQ("Field name", "Field", hfield->getName());
		VASSERT_EQ("Field value", toHex("field data"), toHex(hvalue->getWholeBuffer()));
	}

	void testValueWithEmptyLine()
	{
		vmime::parsingContext ctx;

		const vmime::string buffer = "Field: \r\n\tdata1\r\n\tdata2\r\n\t\r\n\tdata3";

		vmime::shared_ptr <vmime::headerField> hfield =
			vmime::headerField::parseNext(ctx, buffer, 0, buffer.size());

		vmime::shared_ptr <vmime::text> hvalue =
			hfield->getValue <vmime::text>();

		VASSERT_EQ("Field name", "Field", hfield->getName());
		VASSERT_EQ("Field value", "data1 data2 data3", hvalue->getWholeBuffer());
	}

VMIME_TEST_SUITE_END
