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

#ifndef VMIME_EXCEPTION_HPP_INCLUDED
#define VMIME_EXCEPTION_HPP_INCLUDED


#include "config.hpp"
#include "base.hpp"
#include "utility/path.hpp"


namespace vmime
{


class exception
{
private:

	string m_what;
	exception* m_other;

	exception();

public:

	exception(const string& what, const exception& other = NO_EXCEPTION);
	virtual ~exception();

	/** Return a description of the error.
	  *
	  * @return error message
	  */
	const string what() const throw();

	/** Return the next exception in the chain (encapsuled exception).
	  *
	  * @return next exception in the chain
	  */
	const exception* other() const;

	/** Return a name identifying the exception.
	  *
	  * @return exception name
	  */
	virtual const string name() const;

protected:

	static const exception NO_EXCEPTION;

	virtual exception* clone() const;
};



/** List of all VMime exceptions. */

namespace exceptions
{


class bad_field_type : public vmime::exception
{
public:

	bad_field_type(const exception& other = NO_EXCEPTION);
	~bad_field_type() throw();

	exception* clone() const;
	const string name() const;
};


class charset_conv_error : public vmime::exception
{
public:

	charset_conv_error(const exception& other = NO_EXCEPTION);
	~charset_conv_error() throw();

	exception* clone() const;
	const string name() const;
};


class no_encoder_available : public vmime::exception
{
public:

	no_encoder_available(const exception& other = NO_EXCEPTION);
	~no_encoder_available() throw();

	exception* clone() const;
	const string name() const;
};


class no_such_parameter : public vmime::exception
{
public:

	no_such_parameter(const string& name, const exception& other = NO_EXCEPTION);
	~no_such_parameter() throw();

	exception* clone() const;
	const string name() const;
};


class no_such_field : public vmime::exception
{
public:

	no_such_field(const exception& other = NO_EXCEPTION);
	~no_such_field() throw();

	exception* clone() const;
	const string name() const;
};


class no_such_part : public vmime::exception
{
public:

	no_such_part(const exception& other = NO_EXCEPTION);
	~no_such_part() throw();

	exception* clone() const;
	const string name() const;
};


class no_such_mailbox : public vmime::exception
{
public:

	no_such_mailbox(const exception& other = NO_EXCEPTION);
	~no_such_mailbox() throw();

	exception* clone() const;
	const string name() const;
};


class no_such_address : public vmime::exception
{
public:

	no_such_address(const exception& other = NO_EXCEPTION);
	~no_such_address() throw();

	exception* clone() const;
	const string name() const;
};


class open_file_error : public vmime::exception
{
public:

	open_file_error(const exception& other = NO_EXCEPTION);
	~open_file_error() throw();

	exception* clone() const;
	const string name() const;
};


class no_factory_available : public vmime::exception
{
public:

	no_factory_available(const exception& other = NO_EXCEPTION);
	~no_factory_available() throw();

	exception* clone() const;
	const string name() const;
};


class no_platform_dependant_handler : public vmime::exception
{
public:

	no_platform_dependant_handler(const exception& other = NO_EXCEPTION);
	~no_platform_dependant_handler() throw();

	exception* clone() const;
	const string name() const;
};


/** No expeditor specified.
  */

class no_expeditor : public vmime::exception
{
public:

	no_expeditor(const exception& other = NO_EXCEPTION);
	~no_expeditor() throw();

	exception* clone() const;
	const string name() const;
};


/** No recipient specified.
  */

class no_recipient : public vmime::exception
{
public:

	no_recipient(const exception& other = NO_EXCEPTION);
	~no_recipient() throw();

	exception* clone() const;
	const string name() const;
};


class no_object_found : public vmime::exception
{
public:

	no_object_found(const exception& other = NO_EXCEPTION);
	~no_object_found() throw();

	exception* clone() const;
	const string name() const;
};


/** There is no property with that name in the set.
  */

class no_such_property : public vmime::exception
{
public:

	no_such_property(const string& name, const exception& other = NO_EXCEPTION);
	~no_such_property() throw();

	exception* clone() const;
	const string name() const;
};


/** Bad type specified when reading property.
  */

class invalid_property_type : public vmime::exception
{
public:

	invalid_property_type(const exception& other = NO_EXCEPTION);
	~invalid_property_type() throw();

	exception* clone() const;
	const string name() const;
};


/** Bad argument was passed to the function.
  */

class invalid_argument : public vmime::exception
{
public:

	invalid_argument(const exception& other = NO_EXCEPTION);
	~invalid_argument() throw();

	exception* clone() const;
	const string name() const;
};



#if VMIME_HAVE_MESSAGING_FEATURES


/** Base class for exceptions thrown by the messaging module.
  */

class messaging_exception : public vmime::exception
{
public:

	messaging_exception(const string& what, const exception& other = NO_EXCEPTION);
	~messaging_exception() throw();

	exception* clone() const;
	const string name() const;
};


/** Error while connecting to the server: this may be a DNS resolution error
  * or a connection error (for example, time-out while connecting).
  */

class connection_error : public messaging_exception
{
public:

	connection_error(const exception& other = NO_EXCEPTION);
	~connection_error() throw();

	exception* clone() const;
	const string name() const;
};


/** Server did not initiated the connection correctly.
  */

class connection_greeting_error : public messaging_exception
{
public:

	connection_greeting_error(const string& response, const exception& other = NO_EXCEPTION);
	~connection_greeting_error() throw();

	const string& response() const;

	exception* clone() const;
	const string name() const;

private:

	string m_response;
};


/** Error while giving credentials to the server (wrong username
  * or password, or wrong authentication method).
  */

class authentication_error : public messaging_exception
{
public:

	authentication_error(const string& response, const exception& other = NO_EXCEPTION);
	~authentication_error() throw();

	const string& response() const;

	exception* clone() const;
	const string name() const;

private:

	string m_response;
};


/** Option not supported.
  */

class unsupported_option : public messaging_exception
{
public:

	unsupported_option(const exception& other = NO_EXCEPTION);
	~unsupported_option() throw();

	exception* clone() const;
	const string name() const;
};


/** No service available for this protocol.
  */

class no_service_available : public messaging_exception
{
public:

	no_service_available(const exception& other = NO_EXCEPTION);
	~no_service_available() throw();

	exception* clone() const;
	const string name() const;
};


/** The current state of the object does not permit to execute the
  * operation (for example, you try to close a folder which is not open).
  */

class illegal_state : public messaging_exception
{
public:

	illegal_state(const string& state, const exception& other = NO_EXCEPTION);
	~illegal_state() throw();

	exception* clone() const;
	const string name() const;
};


/** Folder not found (does not exist).
  */

class folder_not_found : public messaging_exception
{
public:

	folder_not_found(const exception& other = NO_EXCEPTION);
	~folder_not_found() throw();

	exception* clone() const;
	const string name() const;
};


/** Message not found (does not exist).
  */

class message_not_found : public messaging_exception
{
public:

	message_not_found(const exception& other = NO_EXCEPTION);
	~message_not_found() throw();

	exception* clone() const;
	const string name() const;
};


/** Operation not supported by the underlying protocol.
  */

class operation_not_supported : public messaging_exception
{
public:

	operation_not_supported(const exception& other = NO_EXCEPTION);
	~operation_not_supported() throw();

	exception* clone() const;
	const string name() const;
};


/** The operation timed out (time-out delay is elapsed).
  */

class operation_timed_out : public messaging_exception
{
public:

	operation_timed_out(const exception& other = NO_EXCEPTION);
	~operation_timed_out() throw();

	exception* clone() const;
	const string name() const;
};


/** The operation has been cancelled.
  */

class operation_cancelled : public messaging_exception
{
public:

	operation_cancelled(const exception& other = NO_EXCEPTION);
	~operation_cancelled() throw();

	exception* clone() const;
	const string name() const;
};


/** Must call fetchMessage() or fetchHeader() before accessing
  * the requested object.
  */

class unfetched_object : public messaging_exception
{
public:

	unfetched_object(const exception& other = NO_EXCEPTION);
	~unfetched_object() throw();

	exception* clone() const;
	const string name() const;
};


/** The service is not currently connected.
  */

class not_connected : public messaging_exception
{
public:

	not_connected(const exception& other = NO_EXCEPTION);
	~not_connected() throw();

	exception* clone() const;
	const string name() const;
};


/** The service is already connected (must disconnect before).
  */

class already_connected : public messaging_exception
{
public:

	already_connected(const exception& other = NO_EXCEPTION);
	~already_connected() throw();

	exception* clone() const;
	const string name() const;
};


/** Illegal operation: cannot run this operation on the object.
  */

class illegal_operation : public messaging_exception
{
public:

	illegal_operation(const string& msg = "", const exception& other = NO_EXCEPTION);
	~illegal_operation() throw();

	exception* clone() const;
	const string name() const;
};


/** Command error: operation failed (this is specific to the underlying protocol).
  */

class command_error : public messaging_exception
{
public:

	command_error(const string& command, const string& response, const string& desc = "", const exception& other = NO_EXCEPTION);
	~command_error() throw();

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependant.
	  *
	  * @return command name (protocol-dependant)
	  */
	const string& command() const;

	/** Return the invalid response line.
	  * The meaning is protocol-dependant.
	  *
	  * @return response line (protocol-dependant)
	  */
	const string& response() const;

	exception* clone() const;
	const string name() const;

private:

	string m_command;
	string m_response;
};


/** The server returned an invalid response.
  */

class invalid_response : public messaging_exception
{
public:

	invalid_response(const string& command, const string& response, const exception& other = NO_EXCEPTION);
	~invalid_response() throw();

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependant.
	  *
	  * @return command name (protocol-dependant)
	  */
	const string& command() const;

	/** Return the invalid response line.
	  * The meaning is protocol-dependant.
	  *
	  * @return response line (protocol-dependant)
	  */
	const string& response() const;

	exception* clone() const;
	const string name() const;

private:

	string m_command;
	string m_response;
};


/** Partial fetch is not supported by the underlying protocol.
  */

class partial_fetch_not_supported : public messaging_exception
{
public:

	partial_fetch_not_supported(const exception& other = NO_EXCEPTION);
	~partial_fetch_not_supported() throw();

	exception* clone() const;
	const string name() const;
};


/** The URL is malformed.
  */

class malformed_url : public messaging_exception
{
public:

	malformed_url(const string& error, const exception& other = NO_EXCEPTION);
	~malformed_url() throw();

	exception* clone() const;
	const string name() const;
};


/** Folder name is invalid.
  */

class invalid_folder_name : public messaging_exception
{
public:

	invalid_folder_name(const string& error = "", const exception& other = NO_EXCEPTION);
	~invalid_folder_name() throw();

	exception* clone() const;
	const string name() const;
};


#endif // VMIME_HAVE_MESSAGING_FEATURES


#if VMIME_HAVE_FILESYSTEM_FEATURES


/** Base class for exceptions thrown by the filesystem features.
  */

class filesystem_exception : public vmime::exception
{
public:

	filesystem_exception(const string& what, const utility::path& path, const exception& other = NO_EXCEPTION);
	~filesystem_exception() throw();

	/** Return the full path of the file have thrown the exception.
	  *
	  * @return full path of the file/directory
	  */
	const utility::path& path() const;

	exception* clone() const;
	const string name() const;

private:

	const utility::path m_path;
};


/** File is not a directory.
  */

class not_a_directory : public filesystem_exception
{
public:

	not_a_directory(const utility::path& path, const exception& other = NO_EXCEPTION);
	~not_a_directory() throw();

	exception* clone() const;
	const string name() const;
};


/** File not found.
  */

class file_not_found : public filesystem_exception
{
public:

	file_not_found(const utility::path& path, const exception& other = NO_EXCEPTION);
	~file_not_found() throw();

	exception* clone() const;
	const string name() const;
};


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


} // exceptions


} // vmime


#endif // VMIME_EXCEPTION_HPP_INCLUDED
