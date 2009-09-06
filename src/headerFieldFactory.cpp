//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/headerFieldFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/mailboxList.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/text.hpp"
#include "vmime/path.hpp"
#include "vmime/relay.hpp"
#include "vmime/encoding.hpp"
#include "vmime/disposition.hpp"
#include "vmime/messageIdSequence.hpp"

#include "vmime/contentTypeField.hpp"
#include "vmime/contentDispositionField.hpp"
#include "vmime/mailboxField.hpp"


namespace vmime
{


headerFieldFactory::headerFieldFactory()
{
	// Register parameterized fields
	registerField <contentTypeField>(vmime::fields::CONTENT_TYPE);
	registerField <parameterizedHeaderField>(vmime::fields::CONTENT_TRANSFER_ENCODING);
	registerField <contentDispositionField>(vmime::fields::CONTENT_DISPOSITION);

	registerField <mailboxField>(vmime::fields::FROM);
	registerField <mailboxField>(vmime::fields::SENDER);
	registerField <mailboxField>(vmime::fields::REPLY_TO);
	registerField <mailboxField>(vmime::fields::DELIVERED_TO);

	// Register standard field values
	registerFieldValue <mailbox>(vmime::fields::FROM);
	registerFieldValue <addressList>(vmime::fields::TO);
	registerFieldValue <addressList>(vmime::fields::CC);
	registerFieldValue <addressList>(vmime::fields::BCC);
	registerFieldValue <mailbox>(vmime::fields::SENDER);
	registerFieldValue <datetime>(vmime::fields::DATE);
	registerFieldValue <relay>(vmime::fields::RECEIVED);
	registerFieldValue <text>(vmime::fields::SUBJECT);
	registerFieldValue <mailbox>(vmime::fields::REPLY_TO);
	registerFieldValue <mailbox>(vmime::fields::DELIVERED_TO);
	registerFieldValue <text>(vmime::fields::ORGANIZATION);
	registerFieldValue <text>(vmime::fields::USER_AGENT);
	registerFieldValue <path>(vmime::fields::RETURN_PATH);
	registerFieldValue <mediaType>(vmime::fields::CONTENT_TYPE);
	registerFieldValue <encoding>(vmime::fields::CONTENT_TRANSFER_ENCODING);
	registerFieldValue <text>(vmime::fields::CONTENT_DESCRIPTION);
	registerFieldValue <text>(vmime::fields::MIME_VERSION);
	registerFieldValue <contentDisposition>(vmime::fields::CONTENT_DISPOSITION);
	registerFieldValue <messageId>(vmime::fields::CONTENT_ID);
	registerFieldValue <messageId>(vmime::fields::MESSAGE_ID);
	registerFieldValue <text>(vmime::fields::CONTENT_LOCATION);
	registerFieldValue <messageIdSequence>(vmime::fields::IN_REPLY_TO);
	registerFieldValue <messageIdSequence>(vmime::fields::REFERENCES);

	registerFieldValue <messageId>(vmime::fields::ORIGINAL_MESSAGE_ID);
	registerFieldValue <disposition>(vmime::fields::DISPOSITION);
	registerFieldValue <mailboxList>(vmime::fields::DISPOSITION_NOTIFICATION_TO);
}


headerFieldFactory::~headerFieldFactory()
{
}


headerFieldFactory* headerFieldFactory::getInstance()
{
	static headerFieldFactory instance;
	return (&instance);
}


ref <headerField> headerFieldFactory::create
	(const string& name, const string& body)
{
	NameMap::const_iterator pos = m_nameMap.find(utility::stringUtils::toLower(name));
	ref <headerField> field = NULL;

	if (pos != m_nameMap.end())
		field = ((*pos).second)();
	else
		field = registerer <headerField, headerField>::creator();

	field->setName(name);
	field->setValue(createValue(name));

	if (body != NULL_STRING)
		field->parse(body);

	return field;
}


ref <headerFieldValue> headerFieldFactory::createValue(const string& fieldName)
{
	ValueMap::const_iterator pos = m_valueMap.find
		(utility::stringUtils::toLower(fieldName));

	ref <headerFieldValue> value = NULL;

	if (pos != m_valueMap.end())
		value = ((*pos).second)();
	else
		value = registerer <headerFieldValue, text>::creator();

	return value;
}


} // vmime

