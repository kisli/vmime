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
// You should.have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
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

// MDN
#include "vmime/mdn/MDNHelper.hpp"

// Misc
#include "vmime/misc/importanceHelper.hpp"

// Property set
#include "vmime/propertySet.hpp"

// Utilities
#include "vmime/utility/datetimeUtils.hpp"
#include "vmime/utility/filteredStream.hpp"

// Security
#include "vmime/security/authenticator.hpp"
#include "vmime/security/defaultAuthenticator.hpp"

// Security/digest
#include "vmime/security/digest/messageDigestFactory.hpp"

// Security/SASL
#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/SASLAuthenticator.hpp"
	#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"
	#include "vmime/security/sasl/SASLContext.hpp"
	#include "vmime/security/sasl/SASLSession.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

// Messaging features
#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/net/socket.hpp"

	#include "vmime/net/service.hpp"
	#include "vmime/net/store.hpp"
	#include "vmime/net/transport.hpp"

	#include "vmime/net/session.hpp"

	#include "vmime/net/folder.hpp"
	#include "vmime/net/message.hpp"
#endif // VMIME_HAVE_MESSAGING_FEATURES

// Net/TLS
#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/net/tls/certificate.hpp"
	#include "vmime/net/tls/certificateChain.hpp"
	#include "vmime/net/tls/certificateVerifier.hpp"

	#include "vmime/net/tls/X509Certificate.hpp"

	#include "vmime/net/tls/defaultCertificateVerifier.hpp"

	#include "vmime/net/tls/TLSSession.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT


#endif // VMIME_INCLUDED
