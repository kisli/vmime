//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/headerFieldFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/standardFields.hpp"

#include "vmime/mailboxField.hpp"
#include "vmime/contentTypeField.hpp"
#include "vmime/contentDispositionField.hpp"


namespace vmime
{


headerFieldFactory::headerFieldFactory()
{
	// Register some default fields
	registerName <mailboxField>(vmime::fields::FROM);
	registerName <addressListField>(vmime::fields::TO);
	registerName <addressListField>(vmime::fields::CC);
	registerName <addressListField>(vmime::fields::BCC);
	registerName <mailboxField>(vmime::fields::SENDER);
	registerName <dateField>(vmime::fields::DATE);
	registerName <relayField>(vmime::fields::RECEIVED);
	registerName <textField>(vmime::fields::SUBJECT);
	registerName <mailboxField>(vmime::fields::REPLY_TO);
	registerName <mailboxField>(vmime::fields::DELIVERED_TO);
	registerName <textField>(vmime::fields::ORGANIZATION);
	registerName <textField>(vmime::fields::USER_AGENT);
	registerName <pathField>(vmime::fields::RETURN_PATH);
	registerName <contentTypeField>(vmime::fields::CONTENT_TYPE);
	registerName <contentEncodingField>(vmime::fields::CONTENT_TRANSFER_ENCODING);
	registerName <textField>(vmime::fields::CONTENT_DESCRIPTION);
	registerName <defaultField>(vmime::fields::MIME_VERSION);
	registerName <contentDispositionField>(vmime::fields::CONTENT_DISPOSITION);
	registerName <messageIdField>(vmime::fields::CONTENT_ID);
	registerName <messageIdField>(vmime::fields::MESSAGE_ID);
	registerName <defaultField>(vmime::fields::CONTENT_LOCATION);
	registerName <messageIdField>(vmime::fields::IN_REPLY_TO);

	registerName <messageIdField>(vmime::fields::ORIGINAL_MESSAGE_ID);
	registerName <dispositionField>(vmime::fields::DISPOSITION);
	registerName <mailboxListField>(vmime::fields::DISPOSITION_NOTIFICATION_TO);
}


headerFieldFactory::~headerFieldFactory()
{
}


headerFieldFactory* headerFieldFactory::getInstance()
{
	static headerFieldFactory instance;
	return (&instance);
}


headerField* headerFieldFactory::create
	(const string& name, const string& body)
{
	NameMap::const_iterator pos = m_nameMap.find(utility::stringUtils::toLower(name));
	headerField* field = NULL;

	if (pos != m_nameMap.end())
	{
		field = ((*pos).second)();
	}
	else
	{
		field = registerer <defaultField>::creator();
	}

	field->m_name = name;

	if (body != NULL_STRING)
		field->parse(body);

	return (field);
}


} // vmime
