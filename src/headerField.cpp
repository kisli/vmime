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

#include "headerField.hpp"
#include "headerFieldFactory.hpp"


namespace vmime
{


headerField::headerField()
	: m_type(Custom), m_name("Undefined")
{
}


headerField::headerField(const string& fieldName)
	: m_type(Custom), m_name(fieldName)
{
}


headerField::~headerField()
{
}


headerField* headerField::clone() const
{
	headerField* field = NULL;

	if (m_type == Custom)
		field = headerFieldFactory::getInstance()->create(m_name);
	else
		field = headerFieldFactory::getInstance()->create(m_type);

	field->copyFrom(*this);

	return (field);
}


const bool headerField::operator<(const headerField& field) const
{
	return (m_type < field.m_type);
}


headerField& headerField::operator=(const headerField& field)
{
	copyFrom(field);
	return (*this);
}


void headerField::copyFrom(const headerField& field)
{
	m_type = field.m_type;
	m_name = field.m_name;
}


void headerField::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	if (m_type == Custom)
	{
		os << m_name + ": ";

		if (newLinePos)
			*newLinePos = curLinePos + m_name.length() + 2;
	}
	else
	{
		const string name = typeToName(m_type);

		os << name + ": ";

		if (newLinePos)
			*newLinePos = curLinePos + name.length() + 2;
	}
}


/** Return the field type corresponding to the specified name.
  *
  * @param name field name
  * @return field type (see headerField::Types) or headerField::custom
  * if this is a custom field
  */

const headerField::Types headerField::nameToType(const string& name)
{
	switch (name[0])
	{
	case 'B':
	case 'b':
	{
		if (isStringEqualNoCase(name, "bcc", 3)) return (Bcc);
		break;
	}
	case 'C':
	case 'c':
	{
		if (isStringEqualNoCase(name, "cc", 2)) return (Cc);
		else if (isStringEqualNoCase(name, "content-type", 12)) return (ContentType);
		else if (isStringEqualNoCase(name, "content-transfer-encoding", 25)) return (ContentTransferEncoding);
		else if (isStringEqualNoCase(name, "content-description", 19)) return (ContentDescription);
		else if (isStringEqualNoCase(name, "content-disposition", 19)) return (ContentDisposition);
		else if (isStringEqualNoCase(name, "content-id", 10)) return (ContentId);
		else if (isStringEqualNoCase(name, "content-location", 16)) return (ContentLocation);
		break;
	}
	case 'd':
	case 'D':
	{
		if (isStringEqualNoCase(name, "date", 4)) return (Date);
		else if (isStringEqualNoCase(name, "delivered-to", 12)) return (DeliveredTo);
		break;
	}
	case 'f':
	case 'F':
	{
		if (isStringEqualNoCase(name, "from", 4)) return (From);
		break;
	}
	case 'm':
	case 'M':
	{
		if (isStringEqualNoCase(name, "mime-version", 12)) return (MimeVersion);
		else if (isStringEqualNoCase(name, "message-id", 10)) return (MessageId);
		break;
	}
	case 'o':
	case 'O':
	{
		if (isStringEqualNoCase(name, "organization", 12)) return (Organization);
		break;
	}
	case 'r':
	case 'R':
	{
		if (isStringEqualNoCase(name, "received", 8)) return (Received);
		else if (isStringEqualNoCase(name, "reply-to", 8)) return (ReplyTo);
		else if (isStringEqualNoCase(name, "return-path", 11)) return (ReturnPath);
		break;
	}
	case 's':
	case 'S':
	{
		if (isStringEqualNoCase(name, "sender", 6)) return (Sender);
		else if (isStringEqualNoCase(name, "subject", 7)) return (Subject);
		break;
	}
	case 't':
	case 'T':
	{
		if (isStringEqualNoCase(name, "to", 2)) return (To);
		break;
	}
	case 'u':
	case 'U':
	{
		if (isStringEqualNoCase(name, "user-agent", 10)) return (UserAgent);
		break;
	}

	}

	return (Custom);
}


/** Return the name for the specified field type.
  * Eg: returns "From" for headerField::From.
  *
  * @param type field type
  * @return name for the specified field type
  */

const string headerField::typeToName(const Types type)
{
	switch (type)
	{
	case From: return "From";
	case Sender: return "Sender";
	case To: return "To";
	case Cc: return "Cc";
	case Bcc: return "Bcc";
	case Date: return "Date";
	case Received: return "Received";
	case Subject: return "Subject";
	case ReplyTo: return "Reply-To";
	case Organization: return "Organization";
	case DeliveredTo: return "Delivered-To";
	case UserAgent: return "User-Agent";
	case ReturnPath: return "Return-Path";
	case ContentType: return "Content-Type";
	case ContentTransferEncoding: return "Content-Transfer-Encoding";
	case ContentDescription: return "Content-Description";
	case MimeVersion: return "Mime-Version";
	case ContentDisposition: return "Content-Disposition";
	case ContentId: return "Content-Id";
	case MessageId: return "Message-Id";
	case ContentLocation: return "Content-Location";

	case Custom:
	case Last:
		return "?";
	};

	return "?";
}


/** Return the type of this field.
  *
  * @return field type (see headerField::Types)
  */

const headerField::Types headerField::type() const
{
	return (m_type);
}


/** Return the name of this field.
  *
  * @return field name
  */

const string headerField::name() const
{
	return ((m_type == Custom) ? m_name : typeToName(m_type));
}


/** Check whether this field is a custom field.
  *
  * @return true if the field is a custom field, false otherwise
  */

const bool headerField::isCustom() const
{
	return (m_type == Custom);
}


} // vmime
