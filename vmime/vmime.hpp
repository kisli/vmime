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
// You should.have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_INCLUDED
#define VMIME_INCLUDED


// Configuration
#include "vmime/config.hpp"

// Base definitions
#include "vmime/base.hpp"
#include "vmime/exception.hpp"
#include "vmime/options.hpp"
#include "vmime/platformDependant.hpp"

// Base components
#include "vmime/dateTime.hpp"
#include "vmime/message.hpp"
#include "vmime/bodyPart.hpp"
#include "vmime/charset.hpp"
#include "vmime/text.hpp"
#include "vmime/encoding.hpp"
#include "vmime/contentDisposition.hpp"
#include "vmime/mailbox.hpp"
#include "vmime/mailboxGroup.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/addressList.hpp"
#include "vmime/mediaType.hpp"
#include "vmime/messageId.hpp"
#include "vmime/relay.hpp"
#include "vmime/disposition.hpp"

#include "vmime/emptyContentHandler.hpp"
#include "vmime/stringContentHandler.hpp"
#include "vmime/streamContentHandler.hpp"

// Message components
#include "vmime/message.hpp"

// Header fields
#include "vmime/headerFieldFactory.hpp"
#include "vmime/mailboxField.hpp"
#include "vmime/parameterizedHeaderField.hpp"
#include "vmime/standardFields.hpp"

// Encoders
#include "vmime/encoderFactory.hpp"

// Message builder/parser
#include "vmime/messageBuilder.hpp"
#include "vmime/messageParser.hpp"

#include "vmime/fileAttachment.hpp"
#include "vmime/defaultAttachment.hpp"

#include "vmime/plainTextPart.hpp"
#include "vmime/htmlTextPart.hpp"

// Property set
#include "vmime/propertySet.hpp"

// Utilities
#include "vmime/utility/datetimeUtils.hpp"

// Messaging features
#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/messaging/socket.hpp"

	#include "vmime/messaging/service.hpp"
	#include "vmime/messaging/store.hpp"
	#include "vmime/messaging/transport.hpp"

	#include "vmime/messaging/session.hpp"
	#include "vmime/messaging/authenticator.hpp"
	#include "vmime/messaging/defaultAuthenticator.hpp"
	#include "vmime/messaging/simpleAuthenticator.hpp"

	#include "vmime/messaging/folder.hpp"
	#include "vmime/messaging/message.hpp"
#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_INCLUDED
