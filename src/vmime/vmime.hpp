//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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
#include "config.hpp"

// Base definitions
#include "base.hpp"
#include "exception.hpp"
#include "platform.hpp"

// Base components
#include "dateTime.hpp"
#include "message.hpp"
#include "bodyPart.hpp"
#include "charset.hpp"
#include "text.hpp"
#include "encoding.hpp"
#include "contentDisposition.hpp"
#include "emailAddress.hpp"
#include "mailbox.hpp"
#include "mailboxGroup.hpp"
#include "mailboxList.hpp"
#include "addressList.hpp"
#include "mediaType.hpp"
#include "messageId.hpp"
#include "messageIdSequence.hpp"
#include "relay.hpp"
#include "disposition.hpp"
#include "path.hpp"

#include "emptyContentHandler.hpp"
#include "fileContentHandler.hpp"
#include "stringContentHandler.hpp"
#include "streamContentHandler.hpp"

#include "generationContext.hpp"
#include "parsingContext.hpp"

// Message components
#include "message.hpp"

// Header fields
#include "headerFieldFactory.hpp"
#include "mailboxField.hpp"
#include "parameterizedHeaderField.hpp"

// Encoders
#include "utility/encoder/encoderFactory.hpp"

// Streams
#include "utility/filteredStream.hpp"
#include "utility/inputStream.hpp"
#include "utility/inputStreamAdapter.hpp"
#include "utility/inputStreamByteBufferAdapter.hpp"
#include "utility/inputStreamPointerAdapter.hpp"
#include "utility/inputStreamSocketAdapter.hpp"
#include "utility/inputStreamStringAdapter.hpp"
#include "utility/outputStream.hpp"
#include "utility/outputStreamAdapter.hpp"
#include "utility/outputStreamByteArrayAdapter.hpp"
#include "utility/outputStreamSocketAdapter.hpp"
#include "utility/outputStreamStringAdapter.hpp"
#include "utility/streamUtils.hpp"

// Message builder/parser
#include "messageBuilder.hpp"
#include "messageParser.hpp"

#include "fileAttachment.hpp"
#include "defaultAttachment.hpp"
#include "messageAttachment.hpp"

#include "plainTextPart.hpp"
#include "htmlTextPart.hpp"

#include "attachmentHelper.hpp"

// MDN
#include "mdn/MDNHelper.hpp"

// Misc
#include "misc/importanceHelper.hpp"

// Property set
#include "propertySet.hpp"

// Utilities
#include "utility/datetimeUtils.hpp"
#include "utility/filteredStream.hpp"
#include "charsetConverter.hpp"

// Security
#include "security/authenticator.hpp"
#include "security/defaultAuthenticator.hpp"

// Security/digest
#include "security/digest/messageDigestFactory.hpp"

// Security/SASL
#if VMIME_HAVE_SASL_SUPPORT
	#include "security/sasl/SASLAuthenticator.hpp"
	#include "security/sasl/defaultSASLAuthenticator.hpp"
	#include "security/sasl/SASLContext.hpp"
	#include "security/sasl/SASLSession.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

// Messaging features
#if VMIME_HAVE_MESSAGING_FEATURES
	#include "net/socket.hpp"

	#include "net/serviceFactory.hpp"
	#include "net/store.hpp"
	#include "net/transport.hpp"

	#include "net/session.hpp"

	#include "net/folder.hpp"
	#include "net/message.hpp"
#endif // VMIME_HAVE_MESSAGING_FEATURES

// Net/TLS
#if VMIME_HAVE_TLS_SUPPORT
	#include "security/cert/certificate.hpp"
	#include "security/cert/certificateChain.hpp"
	#include "security/cert/certificateVerifier.hpp"

	#include "security/cert/X509Certificate.hpp"

	#include "security/cert/defaultCertificateVerifier.hpp"

	#include "net/tls/TLSSession.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT


#endif // VMIME_INCLUDED
