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

#ifndef VMIME_NET_SMTP_SMTPEXCEPTIONS_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPEXCEPTIONS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/exception.hpp"
#include "vmime/base.hpp"

#include "vmime/net/smtp/SMTPResponse.hpp"


namespace vmime {
namespace net {
namespace smtp {


/** SMTP Command error: a SMTP command failed.
  */
class VMIME_EXPORT SMTPCommandError : public exceptions::command_error {

public:

	SMTPCommandError(
		const string& command,
		const string& response,
		const string& desc,
		const int statusCode,
		const SMTPResponse::enhancedStatusCode& extendedStatusCode,
		const exception& other = NO_EXCEPTION
	);

	SMTPCommandError(
		const string& command,
		const string& response,
		const int statusCode, const SMTPResponse::enhancedStatusCode& extendedStatusCode,
		const exception& other = NO_EXCEPTION
	);

	~SMTPCommandError() throw();

	/** Returns the SMTP status code for this error.
	  *
	  * @return status code (protocol-dependent)
	  */
	int statusCode() const;

	/** Returns the extended status code (following RFC-3463) for this
	  * error, if available.
	  *
	  * @return status code
	  */
	SMTPResponse::enhancedStatusCode extendedStatusCode() const;


	exception* clone() const;
	const char* name() const throw();

private:

	int m_status;
	SMTPResponse::enhancedStatusCode m_exStatus;
};


/** SMTP error: message size exceeds maximum server limits.
  * This is a permanent error.
  */
class VMIME_EXPORT SMTPMessageSizeExceedsMaxLimitsException : public exceptions::net_exception {

public:

	SMTPMessageSizeExceedsMaxLimitsException(const exception& other = NO_EXCEPTION);
	~SMTPMessageSizeExceedsMaxLimitsException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: message size exceeds current server limits.
  * This is a temporary error (you may retry later).
  */
class VMIME_EXPORT SMTPMessageSizeExceedsCurLimitsException : public exceptions::net_exception {

public:

	SMTPMessageSizeExceedsCurLimitsException(const exception& other = NO_EXCEPTION);
	~SMTPMessageSizeExceedsCurLimitsException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: a required extension is not supported by the server.
  */
class VMIME_EXPORT SMTPExtensionNotSupportedException : public exceptions::net_exception {

public:

	SMTPExtensionNotSupportedException(const string& what, const exception& other = NO_EXCEPTION);
	~SMTPExtensionNotSupportedException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: RFC-1891 DSN extension is not supported by the server.
  */
class VMIME_EXPORT SMTPDSNExtensionNotSupportedException : public SMTPExtensionNotSupportedException {

public:

	SMTPDSNExtensionNotSupportedException(const exception& other = NO_EXCEPTION);
	~SMTPDSNExtensionNotSupportedException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: RFC-6531 SMTPUTF8 extension is not supported by the server,
  * whereas it is needed to send a message with non-ASCII addresses.
  * A server which does not announce 8BITMIME along with SMTPUTF8 does not
  * support it either (RFC 6531, section 3.1).
  */
class VMIME_EXPORT SMTPUTF8ExtensionNotSupportedException : public SMTPExtensionNotSupportedException {

public:

	SMTPUTF8ExtensionNotSupportedException(const exception& other = NO_EXCEPTION);
	~SMTPUTF8ExtensionNotSupportedException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: RFC-6152 8BITMIME extension is not supported by the server,
  * whereas it is needed to send a message which contains 8-bit data.
  */
class VMIME_EXPORT SMTP8BITMIMEExtensionNotSupportedException : public SMTPExtensionNotSupportedException {

public:

	SMTP8BITMIMEExtensionNotSupportedException(const exception& other = NO_EXCEPTION);
	~SMTP8BITMIMEExtensionNotSupportedException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: RFC-3030 BINARYMIME extension is not supported by the server,
  * or cannot be used (it can only be used with the CHUNKING extension),
  * whereas it is needed to send a message which contains binary data.
  */
class VMIME_EXPORT SMTPBINARYMIMEExtensionNotSupportedException : public SMTPExtensionNotSupportedException {

public:

	SMTPBINARYMIMEExtensionNotSupportedException(const exception& other = NO_EXCEPTION);
	~SMTPBINARYMIMEExtensionNotSupportedException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: the message contains binary data (NUL, or lines longer than
  * 998 octets), but it was not sent with the BODY=BINARYMIME parameter
  * (RFC 3030), as VMime could not know it in advance. The message has not
  * been sent: send it again, telling that it contains binary data with
  * SMTPSendOptions::setBinaryMessage().
  */
class VMIME_EXPORT SMTPUndeclaredBinaryDataException : public exceptions::net_exception {

public:

	SMTPUndeclaredBinaryDataException(const exception& other = NO_EXCEPTION);
	~SMTPUndeclaredBinaryDataException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: the message contains 8-bit data, but it was not sent with
  * the BODY=8BITMIME parameter (RFC 6152), as VMime could not know it in
  * advance. The message has not been sent: send it again, telling that it
  * contains 8-bit data with SMTPSendOptions::set8bitMessage().
  */
class VMIME_EXPORT SMTPUndeclared8bitDataException : public exceptions::net_exception {

public:

	SMTPUndeclared8bitDataException(const exception& other = NO_EXCEPTION);
	~SMTPUndeclared8bitDataException() throw();

	exception* clone() const;
	const char* name() const throw();
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPEXCEPTIONS_HPP_INCLUDED

