//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "headerFieldFactory.hpp"
#include "exception.hpp"

#include "defaultField.hpp"

#include "mailboxField.hpp"
#include "addressListField.hpp"
#include "addressListField.hpp"
#include "addressListField.hpp"
#include "mailboxField.hpp"
#include "dateField.hpp"
#include "relayField.hpp"
#include "textField.hpp"
#include "mailboxField.hpp"
#include "contentTypeField.hpp"
#include "contentEncodingField.hpp"
#include "contentDispositionField.hpp"
#include "messageIdField.hpp"


namespace vmime
{


headerFieldFactory::headerFieldFactory()
{
	// Register some default field types
	registerType <mailboxField>(headerField::From);
	registerType <addressListField>(headerField::To);
	registerType <addressListField>(headerField::Cc);
	registerType <addressListField>(headerField::Bcc);
	registerType <mailboxField>(headerField::Sender);
	registerType <dateField>(headerField::Date);
	registerType <relayField>(headerField::Received);
	registerType <textField>(headerField::Subject);
	registerType <mailboxField>(headerField::ReplyTo);
	registerType <mailboxField>(headerField::DeliveredTo);
	registerType <textField>(headerField::Organization);
	registerType <textField>(headerField::UserAgent);
	registerType <mailboxField>(headerField::ReturnPath);
	registerType <contentTypeField>(headerField::ContentType);
	registerType <contentEncodingField>(headerField::ContentTransferEncoding);
	registerType <textField>(headerField::ContentDescription);
	registerType <defaultField>(headerField::MimeVersion);
	registerType <contentDispositionField>(headerField::ContentDisposition);
	registerType <messageIdField>(headerField::ContentId);
	registerType <messageIdField>(headerField::MessageId);
	registerType <defaultField>(headerField::ContentLocation);
}


headerFieldFactory::~headerFieldFactory()
{
}


headerField* headerFieldFactory::create
	(const string& name, const string& body)
{
	const headerField::Types type = headerField::nameToType(name);

	if (type != headerField::Custom)
	{
		return (create(type, name, body));
	}
	else
	{
		NameMap::const_iterator pos = m_nameMap.find(toLower(name));
		headerField* field = NULL;

		if (pos != m_nameMap.end())
		{
			field = ((*pos).second)();
		}
		else
		{
			field = new defaultField;
		}

		field->m_type = headerField::Custom;
		field->m_name = name;

		if (body != NULL_STRING)
			field->parse(body);

		return (field);
	}
}


headerField* headerFieldFactory::create(const headerField::Types type,
	const string& name, const string& body)
{
	if (type == headerField::Custom)
	{
		return (create(name, body));
	}
	else
	{
		TypeMap::const_iterator pos = m_typeMap.find(type);

		if (pos != m_typeMap.end())
		{
			headerField* field = ((*pos).second)();

			field->m_type = type;
			if (name != NULL_STRING) field->m_name = name;
			if (body != NULL_STRING) field->parse(body);

			return (field);
		}
		else
		{
			throw exceptions::bad_field_type();
		}
	}
}


} // vmime
