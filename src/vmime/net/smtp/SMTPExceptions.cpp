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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPExceptions.hpp"


namespace vmime {
namespace net {
namespace smtp {


//
// SMTPCommandError
//

SMTPCommandError::SMTPCommandError(
	const string& command,
	const string& response,
	const string& desc,
	const int statusCode,
    const SMTPResponse::enhancedStatusCode& extendedStatusCode,
    const exception& other
)
	: command_error(command, response, desc, other),
	  m_status(statusCode),
	  m_exStatus(extendedStatusCode) {

}


SMTPCommandError::SMTPCommandError(
	const string& command,
	const string& response,
	const int statusCode,
	const SMTPResponse::enhancedStatusCode& extendedStatusCode,
    const exception& other
)
	: command_error(command, response, "", other),
	  m_status(statusCode),
	  m_exStatus(extendedStatusCode) {

}


SMTPCommandError::~SMTPCommandError() throw() {

}


int SMTPCommandError::statusCode() const {

	return m_status;
}


SMTPResponse::enhancedStatusCode SMTPCommandError::extendedStatusCode() const {

	return m_exStatus;
}


exception* SMTPCommandError::clone() const {

	return new SMTPCommandError(*this);
}


const char* SMTPCommandError::name() const throw() {

	return "SMTPCommandError";
}


//
// SMTPMessageSizeExceedsMaxLimitsException
//

SMTPMessageSizeExceedsMaxLimitsException::SMTPMessageSizeExceedsMaxLimitsException(const exception& other)
	: net_exception("Message size exceeds maximum server limits (permanent error).", other) {

}


SMTPMessageSizeExceedsMaxLimitsException::~SMTPMessageSizeExceedsMaxLimitsException() throw() {

}


exception* SMTPMessageSizeExceedsMaxLimitsException::clone() const {

	return new SMTPMessageSizeExceedsMaxLimitsException(*this);
}


const char* SMTPMessageSizeExceedsMaxLimitsException::name() const throw() {

	return "SMTPMessageSizeExceedsMaxLimitsException";
}


//
// SMTPMessageSizeExceedsCurLimitsException
//

SMTPMessageSizeExceedsCurLimitsException::SMTPMessageSizeExceedsCurLimitsException(const exception& other)
	: net_exception("Message size exceeds current server limits (temporary storage error).", other) {

}


SMTPMessageSizeExceedsCurLimitsException::~SMTPMessageSizeExceedsCurLimitsException() throw() {

}


exception* SMTPMessageSizeExceedsCurLimitsException::clone() const {

	return new SMTPMessageSizeExceedsCurLimitsException(*this);
}


const char* SMTPMessageSizeExceedsCurLimitsException::name() const throw() {

	return "SMTPMessageSizeExceedsCurLimitsException";
}


//
// SMTPExtensionNotSupportedException
//

SMTPExtensionNotSupportedException::SMTPExtensionNotSupportedException(const string &what, const exception& other)
	: net_exception(what.empty() ? "A required extension is not supported by the SMTP server." : what, other) {

}


SMTPExtensionNotSupportedException::~SMTPExtensionNotSupportedException() throw() {

}


exception* SMTPExtensionNotSupportedException::clone() const {

	return new SMTPExtensionNotSupportedException(*this);
}


const char* SMTPExtensionNotSupportedException::name() const throw() {

	return "SMTPExtensionNotSupportedException";
}


//
// SMTPDSNExtensionNotSupportedException
//

SMTPDSNExtensionNotSupportedException::SMTPDSNExtensionNotSupportedException(const exception& other)
	: SMTPExtensionNotSupportedException("RFC-1891 DSN extension is not supported by the SMTP server.", other) {

}


SMTPDSNExtensionNotSupportedException::~SMTPDSNExtensionNotSupportedException() throw() {

}


exception* SMTPDSNExtensionNotSupportedException::clone() const {
	return new SMTPDSNExtensionNotSupportedException(*this);
}


const char* SMTPDSNExtensionNotSupportedException::name() const throw() {

	return "SMTPDSNExtensionNotSupportedException";
}


//
// SMTPUTF8ExtensionNotSupportedException
//

SMTPUTF8ExtensionNotSupportedException::SMTPUTF8ExtensionNotSupportedException(const exception& other)
	: SMTPExtensionNotSupportedException(
		"RFC-6531 SMTPUTF8 extension is not supported by the SMTP server, "
		"but is needed for non-ASCII addresses.", other) {

}


SMTPUTF8ExtensionNotSupportedException::~SMTPUTF8ExtensionNotSupportedException() throw() {

}


exception* SMTPUTF8ExtensionNotSupportedException::clone() const {
	return new SMTPUTF8ExtensionNotSupportedException(*this);
}


const char* SMTPUTF8ExtensionNotSupportedException::name() const throw() {

	return "SMTPUTF8ExtensionNotSupportedException";
}


//
// SMTP8BITMIMEExtensionNotSupportedException
//

SMTP8BITMIMEExtensionNotSupportedException::SMTP8BITMIMEExtensionNotSupportedException(const exception& other)
	: SMTPExtensionNotSupportedException(
		"RFC-6152 8BITMIME extension is not supported by the SMTP server, "
		"but is needed for 8-bit data.", other) {

}


SMTP8BITMIMEExtensionNotSupportedException::~SMTP8BITMIMEExtensionNotSupportedException() throw() {

}


exception* SMTP8BITMIMEExtensionNotSupportedException::clone() const {
	return new SMTP8BITMIMEExtensionNotSupportedException(*this);
}


const char* SMTP8BITMIMEExtensionNotSupportedException::name() const throw() {

	return "SMTP8BITMIMEExtensionNotSupportedException";
}


//
// SMTPBINARYMIMEExtensionNotSupportedException
//

SMTPBINARYMIMEExtensionNotSupportedException::SMTPBINARYMIMEExtensionNotSupportedException(const exception& other)
	: SMTPExtensionNotSupportedException(
		"RFC-3030 BINARYMIME extension (with CHUNKING) is not supported by the "
		"SMTP server, but is needed for binary data.", other) {

}


SMTPBINARYMIMEExtensionNotSupportedException::~SMTPBINARYMIMEExtensionNotSupportedException() throw() {

}


exception* SMTPBINARYMIMEExtensionNotSupportedException::clone() const {
	return new SMTPBINARYMIMEExtensionNotSupportedException(*this);
}


const char* SMTPBINARYMIMEExtensionNotSupportedException::name() const throw() {

	return "SMTPBINARYMIMEExtensionNotSupportedException";
}


//
// SMTPUndeclaredBinaryDataException
//

SMTPUndeclaredBinaryDataException::SMTPUndeclaredBinaryDataException(const exception& other)
	: net_exception(
		"The message contains binary data, which must be declared "
		"with SMTPSendOptions::setBinaryMessage().", other) {

}


SMTPUndeclaredBinaryDataException::~SMTPUndeclaredBinaryDataException() throw() {

}


exception* SMTPUndeclaredBinaryDataException::clone() const {
	return new SMTPUndeclaredBinaryDataException(*this);
}


const char* SMTPUndeclaredBinaryDataException::name() const throw() {

	return "SMTPUndeclaredBinaryDataException";
}


//
// SMTPUndeclared8bitDataException
//

SMTPUndeclared8bitDataException::SMTPUndeclared8bitDataException(const exception& other)
	: net_exception(
		"The message contains 8-bit data, which must be declared "
		"with SMTPSendOptions::set8bitMessage().", other) {

}


SMTPUndeclared8bitDataException::~SMTPUndeclared8bitDataException() throw() {

}


exception* SMTPUndeclared8bitDataException::clone() const {
	return new SMTPUndeclared8bitDataException(*this);
}


const char* SMTPUndeclared8bitDataException::name() const throw() {

	return "SMTPUndeclared8bitDataException";
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
