//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

class VMIME_EXPORT SMTPCommandError : public exceptions::command_error
{
public:

	SMTPCommandError(const string& command, const string& response,
		const string& desc, const int statusCode,
		const SMTPResponse::enhancedStatusCode& extendedStatusCode,
		const exception& other = NO_EXCEPTION);

	SMTPCommandError(const string& command, const string& response,
		const int statusCode, const SMTPResponse::enhancedStatusCode& extendedStatusCode,
		const exception& other = NO_EXCEPTION);

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
	const SMTPResponse::enhancedStatusCode extendedStatusCode() const;


	exception* clone() const;
	const char* name() const throw();

private:

	int m_status;
	SMTPResponse::enhancedStatusCode m_exStatus;
};


/** SMTP error: message size exceeds maximum server limits.
  * This is a permanent error.
  */

class VMIME_EXPORT SMTPMessageSizeExceedsMaxLimitsException : public exceptions::net_exception
{
public:

	SMTPMessageSizeExceedsMaxLimitsException(const exception& other = NO_EXCEPTION);
	~SMTPMessageSizeExceedsMaxLimitsException() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** SMTP error: message size exceeds current server limits.
  * This is a temporary error (you may retry later).
  */

class VMIME_EXPORT SMTPMessageSizeExceedsCurLimitsException : public exceptions::net_exception
{
public:

	SMTPMessageSizeExceedsCurLimitsException(const exception& other = NO_EXCEPTION);
	~SMTPMessageSizeExceedsCurLimitsException() throw();

	exception* clone() const;
	const char* name() const throw();
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPEXCEPTIONS_HPP_INCLUDED

