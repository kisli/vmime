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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_STANDARDFIELDS_HPP_INCLUDED
#define VMIME_STANDARDFIELDS_HPP_INCLUDED


#include "vmime/genericField.hpp"
#include "vmime/parameterizedHeaderField.hpp"
#include "vmime/headerFieldFactory.hpp"

// Inclusion for field value types
#include "vmime/addressList.hpp"
#include "vmime/encoding.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/text.hpp"
#include "vmime/messageId.hpp"
#include "vmime/relay.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/disposition.hpp"
#include "vmime/path.hpp"
#include "vmime/messageIdSequence.hpp"


namespace vmime
{


#define DECLARE_STANDARD_FIELD(fieldClassName, valueTypeClassName) \
	class fieldClassName : public genericField <valueTypeClassName> { \
		friend class vmime::creator; \
		protected: \
			fieldClassName() { } \
			fieldClassName(const fieldClassName&) \
				: headerField(), \
				  genericField <valueTypeClassName>() { /* Not used */ } \
	}

#ifdef VMIME_NO_MULTIPLE_INHERITANCE

	template <class TYPE>
	class genericParameterizedHeaderField : public genericField <TYPE>
	{
	};

	#define DECLARE_STANDARD_FIELD_PARAM(fieldClassName, valueTypeClassName) \
		class fieldClassName : public genericParameterizedHeaderField <valueTypeClassName> { \
			friend class vmime::creator; \
			protected: \
				fieldClassName() { } \
				fieldClassName(const fieldClassName&) \
					: headerField(), \
					  genericParameterizedHeaderField <valueTypeClassName>() { /* Not used */ } \
		}

#else // VMIME_NO_MULTIPLE_INHERITANCE

	#define DECLARE_STANDARD_FIELD_PARAM(fieldClassName, valueTypeClassName) \
		class fieldClassName : public genericField <valueTypeClassName>, \
		                       public parameterizedHeaderField { \
			friend class vmime::creator; \
			protected: \
				fieldClassName() { } \
				fieldClassName(const fieldClassName&) \
					: headerField(), \
					  genericField <valueTypeClassName>(), \
					  parameterizedHeaderField() { /* Not used */ } \
		}

#endif // VMIME_NO_MULTIPLE_INHERITANCE



DECLARE_STANDARD_FIELD(addressListField, addressList);
DECLARE_STANDARD_FIELD_PARAM(contentEncodingField, encoding);
DECLARE_STANDARD_FIELD(dateField, datetime);
DECLARE_STANDARD_FIELD(textField, text);
DECLARE_STANDARD_FIELD(messageIdField, messageId);
DECLARE_STANDARD_FIELD(defaultField, string);
DECLARE_STANDARD_FIELD(relayField, relay);
DECLARE_STANDARD_FIELD(mailboxListField, mailboxList);
DECLARE_STANDARD_FIELD(dispositionField, disposition);
DECLARE_STANDARD_FIELD(pathField, path);
DECLARE_STANDARD_FIELD(messageIdSequenceField, messageIdSequence);


#undef DECLARE_STANDARD_FIELD
#undef DECLARE_STANDARD_FIELD_PARAM


} // vmime


#endif // VMIME_STANDARDFIELDS_HPP_INCLUDED
