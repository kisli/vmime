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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPExceptions.hpp"


namespace vmime {
namespace net {
namespace smtp {


//
// SMTPCommandError
//

SMTPCommandError::SMTPCommandError
	(const string& command, const string& response,
	 const string& desc, const int statusCode,
     const SMTPResponse::enhancedStatusCode& extendedStatusCode,
     const exception& other)
	: command_error(command, response, desc, other),
	  m_status(statusCode), m_exStatus(extendedStatusCode)
{
}


SMTPCommandError::SMTPCommandError
	(const string& command, const string& response,
	 const int statusCode, const SMTPResponse::enhancedStatusCode& extendedStatusCode,
     const exception& other)
	: command_error(command, response, "", other),
	  m_status(statusCode), m_exStatus(extendedStatusCode)
{
}


SMTPCommandError::~SMTPCommandError() throw()
{
}


int SMTPCommandError::statusCode() const
{
	return m_status;
}


const SMTPResponse::enhancedStatusCode SMTPCommandError::extendedStatusCode() const
{
	return m_exStatus;
}


exception* SMTPCommandError::clone() const
{
	return new SMTPCommandError(*this);
}


const char* SMTPCommandError::name() const throw()
{
	return "SMTPCommandError";
}


//
// SMTPMessageSizeExceedsMaxLimitsException
//

SMTPMessageSizeExceedsMaxLimitsException::SMTPMessageSizeExceedsMaxLimitsException(const exception& other)
	: net_exception("Message size exceeds maximum server limits (permanent error).", other)
{
}


SMTPMessageSizeExceedsMaxLimitsException::~SMTPMessageSizeExceedsMaxLimitsException() throw()
{
}


exception* SMTPMessageSizeExceedsMaxLimitsException::clone() const
{
	return new SMTPMessageSizeExceedsMaxLimitsException(*this);
}


const char* SMTPMessageSizeExceedsMaxLimitsException::name() const throw()
{
	return "SMTPMessageSizeExceedsMaxLimitsException";
}


//
// SMTPMessageSizeExceedsCurLimitsException
//

SMTPMessageSizeExceedsCurLimitsException::SMTPMessageSizeExceedsCurLimitsException(const exception& other)
	: net_exception("Message size exceeds current server limits (temporary storage error).", other)
{
}


SMTPMessageSizeExceedsCurLimitsException::~SMTPMessageSizeExceedsCurLimitsException() throw()
{
}


exception* SMTPMessageSizeExceedsCurLimitsException::clone() const
{
	return new SMTPMessageSizeExceedsCurLimitsException(*this);
}


const char* SMTPMessageSizeExceedsCurLimitsException::name() const throw()
{
	return "SMTPMessageSizeExceedsCurLimitsException";
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
