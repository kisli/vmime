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

#ifndef VMIME_STANDARDFIELDS_HPP_INCLUDED
#define VMIME_STANDARDFIELDS_HPP_INCLUDED


#include "genericField.hpp"
#include "parameterizedHeaderField.hpp"
#include "headerFieldFactory.hpp"

// Inclusion for field value types
#include "addressList.hpp"
#include "encoding.hpp"
#include "dateTime.hpp"
#include "text.hpp"
#include "messageId.hpp"
#include "relay.hpp"
#include "mailboxList.hpp"


namespace vmime
{


#define DECLARE_STANDARD_FIELD(fieldClassName, valueTypeClassName) \
	class fieldClassName : public genericField <valueTypeClassName> { \
		friend class headerFieldFactory::registerer <fieldClassName>; \
		protected: \
			fieldClassName() { } \
			fieldClassName(const fieldClassName&) \
				: headerField(), \
				  genericField <valueTypeClassName>() { /* Not used */ } \
	}

#define DECLARE_STANDARD_FIELD_PARAM(fieldClassName, valueTypeClassName) \
	class fieldClassName : public genericField <valueTypeClassName>, \
	                       public parameterizedHeaderField { \
		friend class headerFieldFactory::registerer <fieldClassName>; \
		protected: \
			fieldClassName() { } \
			fieldClassName(const fieldClassName&) \
				: headerField(), \
				  genericField <valueTypeClassName>(), \
				  parameterizedHeaderField() { /* Not used */ } \
	}


DECLARE_STANDARD_FIELD(addressListField, addressList);
DECLARE_STANDARD_FIELD_PARAM(contentEncodingField, encoding);
DECLARE_STANDARD_FIELD(dateField, datetime);
DECLARE_STANDARD_FIELD(textField, text);
DECLARE_STANDARD_FIELD(messageIdField, messageId);
DECLARE_STANDARD_FIELD(defaultField, string);
DECLARE_STANDARD_FIELD(relayField, relay);
DECLARE_STANDARD_FIELD(mailboxListField, mailboxList);


#undef DECLARE_STANDARD_FIELD
#undef DECLARE_STANDARD_FIELD_PARAM


} // vmime


#endif // VMIME_STANDARDFIELDS_HPP_INCLUDED
