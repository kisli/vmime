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

#ifndef VMIME_EXCEPTION_HPP_INCLUDED
#define VMIME_EXCEPTION_HPP_INCLUDED


#include <stdexcept>

#include "vmime/config.hpp"
#include "vmime/base.hpp"
#include "vmime/utility/path.hpp"


namespace vmime
{


/** Base class for VMime exceptions.
  */

class VMIME_EXPORT exception : public std::exception
{
private:

	string m_what;
	exception* m_other;

	exception();

public:

	exception(const string& what, const exception& other = NO_EXCEPTION);
	exception(const exception& e);

	virtual ~exception() throw();

	/** Return a description of the error.
	  *
	  * @return error message
	  */
	const char* what() const throw();

	/** Return a description of the error.
	  *
	  * @return error message
	  */
	const char* what() throw();

	/** Return the next exception in the chain (encapsulated exception).
	  *
	  * @return next exception in the chain
	  */
	const exception* other() const throw();

	/** Return a name identifying the exception.
	  *
	  * @return exception name
	  */
	virtual const char* name() const throw();

	/** Clone this object.
	  *
	  * @return a new copy of this object
	  */
	virtual exception* clone() const;

protected:

	static const exception NO_EXCEPTION;
};



/** List of all VMime exceptions. */

namespace exceptions
{


class VMIME_EXPORT bad_field_value_type : public vmime::exception
{
public:

	bad_field_value_type(const string& fieldName, const exception& other = NO_EXCEPTION);
	~bad_field_value_type() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT charset_conv_error : public vmime::exception
{
public:

	charset_conv_error(const string& what = "", const exception& other = NO_EXCEPTION);
	~charset_conv_error() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** No encoder has been found for the specified encoding name.
  */

class VMIME_EXPORT no_encoder_available : public vmime::exception
{
public:

	no_encoder_available(const string& name, const exception& other = NO_EXCEPTION);
	~no_encoder_available() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** No algorithm has been found for the specified name.
  */

class VMIME_EXPORT no_digest_algorithm_available : public vmime::exception
{
public:

	no_digest_algorithm_available(const string& name, const exception& other = NO_EXCEPTION);
	~no_digest_algorithm_available() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT no_such_field : public vmime::exception
{
public:

	no_such_field(const exception& other = NO_EXCEPTION);
	~no_such_field() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT no_such_part : public vmime::exception
{
public:

	no_such_part(const exception& other = NO_EXCEPTION);
	~no_such_part() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT no_such_message_id : public vmime::exception
{
public:

	no_such_message_id(const exception& other = NO_EXCEPTION);
	~no_such_message_id() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT open_file_error : public vmime::exception
{
public:

	open_file_error(const exception& other = NO_EXCEPTION);
	~open_file_error() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT no_factory_available : public vmime::exception
{
public:

	no_factory_available(const exception& other = NO_EXCEPTION);
	~no_factory_available() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT no_platform_handler : public vmime::exception
{
public:

	no_platform_handler(const exception& other = NO_EXCEPTION);
	~no_platform_handler() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** No expeditor specified.
  */

class VMIME_EXPORT no_expeditor : public vmime::exception
{
public:

	no_expeditor(const exception& other = NO_EXCEPTION);
	~no_expeditor() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** No recipient specified.
  */

class VMIME_EXPORT no_recipient : public vmime::exception
{
public:

	no_recipient(const exception& other = NO_EXCEPTION);
	~no_recipient() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** There is no property with that name in the set.
  */

class VMIME_EXPORT no_such_property : public vmime::exception
{
public:

	no_such_property(const string& name, const exception& other = NO_EXCEPTION);
	~no_such_property() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Bad type specified when reading property.
  */

class VMIME_EXPORT invalid_property_type : public vmime::exception
{
public:

	invalid_property_type(const exception& other = NO_EXCEPTION);
	~invalid_property_type() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Bad argument was passed to the function.
  */

class VMIME_EXPORT invalid_argument : public vmime::exception
{
public:

	invalid_argument(const exception& other = NO_EXCEPTION);
	~invalid_argument() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Underlying operating system error.
  */

class VMIME_EXPORT system_error : public vmime::exception
{
public:

	system_error(const string& what, const exception& other = NO_EXCEPTION);
	~system_error() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The URL is malformed.
  */

class VMIME_EXPORT malformed_url : public vmime::exception
{
public:

	malformed_url(const string& error, const exception& other = NO_EXCEPTION);
	~malformed_url() throw();

	exception* clone() const;
	const char* name() const throw();
};



#if VMIME_HAVE_MESSAGING_FEATURES


/** Base class for exceptions thrown by the networking module.
  */

class VMIME_EXPORT net_exception : public vmime::exception
{
public:

	net_exception(const string& what, const exception& other = NO_EXCEPTION);
	~net_exception() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Alias for 'net_exception' (compatibility with version <= 0.7.1);
  * this is deprecated.
  */
typedef net_exception messaging_exception;


/** Socket error.
  */

class VMIME_EXPORT socket_exception : public net_exception
{
public:

	socket_exception(const string& what = "", const exception& other = NO_EXCEPTION);
	~socket_exception() throw();

	exception* clone() const;
	const char* name() const throw();

};


/** Socket not connected: you are trying to write to/read from a socket which
  * is not connected to a peer.
  */

class VMIME_EXPORT socket_not_connected_exception : public socket_exception
{
public:

	socket_not_connected_exception(const string& what = "", const exception& other = NO_EXCEPTION);
	~socket_not_connected_exception() throw();

	exception* clone() const;
	const char* name() const throw();

};


/** Error while connecting to the server: this may be a DNS resolution error
  * or a connection error (for example, time-out while connecting).
  */

class VMIME_EXPORT connection_error : public socket_exception
{
public:

	connection_error(const string& what = "", const exception& other = NO_EXCEPTION);
	~connection_error() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Server did not initiated the connection correctly.
  */

class VMIME_EXPORT connection_greeting_error : public net_exception
{
public:

	connection_greeting_error(const string& response, const exception& other = NO_EXCEPTION);
	~connection_greeting_error() throw();

	const string& response() const;

	exception* clone() const;
	const char* name() const throw();

private:

	string m_response;
};


/** Error while giving credentials to the server (wrong username
  * or password, or wrong authentication method).
  */

class VMIME_EXPORT authentication_error : public net_exception
{
public:

	authentication_error(const string& response, const exception& other = NO_EXCEPTION);
	~authentication_error() throw();

	const string& response() const;

	exception* clone() const;
	const char* name() const throw();

private:

	string m_response;
};


/** Option not supported.
  */

class VMIME_EXPORT unsupported_option : public net_exception
{
public:

	unsupported_option(const exception& other = NO_EXCEPTION);
	~unsupported_option() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The current state of the object does not permit to execute the
  * operation (for example, you try to close a folder which is not open).
  */

class VMIME_EXPORT illegal_state : public net_exception
{
public:

	illegal_state(const string& state, const exception& other = NO_EXCEPTION);
	~illegal_state() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Folder not found (does not exist).
  */

class VMIME_EXPORT folder_not_found : public net_exception
{
public:

	folder_not_found(const exception& other = NO_EXCEPTION);
	~folder_not_found() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Folder is already open in the same session.
  */

class VMIME_EXPORT folder_already_open : public net_exception
{
public:

	folder_already_open(const exception& other = NO_EXCEPTION);
	~folder_already_open() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Message not found (does not exist).
  */

class VMIME_EXPORT message_not_found : public net_exception
{
public:

	message_not_found(const exception& other = NO_EXCEPTION);
	~message_not_found() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Operation not supported by the underlying protocol.
  */

class VMIME_EXPORT operation_not_supported : public net_exception
{
public:

	operation_not_supported(const exception& other = NO_EXCEPTION);
	~operation_not_supported() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The operation timed out (time-out delay is elapsed).
  */

class VMIME_EXPORT operation_timed_out : public net_exception
{
public:

	operation_timed_out(const exception& other = NO_EXCEPTION);
	~operation_timed_out() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The operation has been cancelled.
  */

class VMIME_EXPORT operation_cancelled : public net_exception
{
public:

	operation_cancelled(const exception& other = NO_EXCEPTION);
	~operation_cancelled() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Must call fetchMessage() or fetchHeader() before accessing
  * the requested object.
  */

class VMIME_EXPORT unfetched_object : public net_exception
{
public:

	unfetched_object(const exception& other = NO_EXCEPTION);
	~unfetched_object() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The service is not currently connected.
  */

class VMIME_EXPORT not_connected : public net_exception
{
public:

	not_connected(const exception& other = NO_EXCEPTION);
	~not_connected() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The service is already connected (must disconnect before).
  */

class VMIME_EXPORT already_connected : public net_exception
{
public:

	already_connected(const exception& other = NO_EXCEPTION);
	~already_connected() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Illegal operation: cannot run this operation on the object.
  */

class VMIME_EXPORT illegal_operation : public net_exception
{
public:

	illegal_operation(const string& msg = "", const exception& other = NO_EXCEPTION);
	~illegal_operation() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Command error: operation failed (this is specific to the underlying protocol).
  */

class VMIME_EXPORT command_error : public net_exception
{
public:

	command_error(const string& command, const string& response, const string& desc = "", const exception& other = NO_EXCEPTION);
	~command_error() throw();

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependent.
	  *
	  * @return command name (protocol-dependent)
	  */
	const string& command() const;

	/** Return the invalid response line.
	  * The meaning is protocol-dependent.
	  *
	  * @return response line (protocol-dependent)
	  */
	const string& response() const;

	exception* clone() const;
	const char* name() const throw();

private:

	string m_command;
	string m_response;
};


/** The server returned an invalid response.
  */

class VMIME_EXPORT invalid_response : public net_exception
{
public:

	invalid_response(const string& command, const string& response, const exception& other = NO_EXCEPTION);
	~invalid_response() throw();

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependent.
	  *
	  * @return command name (protocol-dependent)
	  */
	const string& command() const;

	/** Return the invalid response line.
	  * The meaning is protocol-dependent.
	  *
	  * @return response line (protocol-dependent)
	  */
	const string& response() const;

	exception* clone() const;
	const char* name() const throw();

private:

	string m_command;
	string m_response;
};


/** Partial fetch is not supported by the underlying protocol.
  */

class VMIME_EXPORT partial_fetch_not_supported : public net_exception
{
public:

	partial_fetch_not_supported(const exception& other = NO_EXCEPTION);
	~partial_fetch_not_supported() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** Folder name is invalid.
  */

class VMIME_EXPORT invalid_folder_name : public net_exception
{
public:

	invalid_folder_name(const string& error = "", const exception& other = NO_EXCEPTION);
	~invalid_folder_name() throw();

	exception* clone() const;
	const char* name() const throw();
};


#endif // VMIME_HAVE_MESSAGING_FEATURES


#if VMIME_HAVE_FILESYSTEM_FEATURES


/** Base class for exceptions thrown by the filesystem features.
  */

class VMIME_EXPORT filesystem_exception : public vmime::exception
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
	const char* name() const throw();

private:

	const utility::path m_path;
};


/** File is not a directory.
  */

class VMIME_EXPORT not_a_directory : public filesystem_exception
{
public:

	not_a_directory(const utility::path& path, const exception& other = NO_EXCEPTION);
	~not_a_directory() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** File not found.
  */

class VMIME_EXPORT file_not_found : public filesystem_exception
{
public:

	file_not_found(const utility::path& path, const exception& other = NO_EXCEPTION);
	~file_not_found() throw();

	exception* clone() const;
	const char* name() const throw();
};


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


/** Authentication exception.
  */

class VMIME_EXPORT authentication_exception : public vmime::exception
{
public:

	authentication_exception(const string& what, const exception& other = NO_EXCEPTION);
	~authentication_exception() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** The requested information cannot be provided.
  */

class VMIME_EXPORT no_auth_information : public authentication_exception
{
public:

	no_auth_information(const exception& other = NO_EXCEPTION);
	~no_auth_information() throw();

	exception* clone() const;
	const char* name() const throw();
};


#if VMIME_HAVE_SASL_SUPPORT


/** Base class for exceptions thrown by SASL module.
  */

class VMIME_EXPORT sasl_exception : public authentication_exception
{
public:

	sasl_exception(const string& what, const exception& other = NO_EXCEPTION);
	~sasl_exception() throw();

	exception* clone() const;
	const char* name() const throw();
};


/** No mechanism is registered with the specified name.
  */

class VMIME_EXPORT no_such_mechanism : public sasl_exception
{
public:

	no_such_mechanism(const string& name, const exception& other = NO_EXCEPTION);
	~no_such_mechanism() throw();

	exception* clone() const;
	const char* name() const throw();
};


#endif // VMIME_HAVE_SASL_SUPPORT


#if VMIME_HAVE_TLS_SUPPORT


/** Base class for exceptions thrown by TLS module.
  */

class VMIME_EXPORT tls_exception : public vmime::exception
{
public:

	tls_exception(const string& what, const exception& other = NO_EXCEPTION);
	~tls_exception() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT certificate_exception : public tls_exception
{
public:

	certificate_exception(const string& what, const exception& other = NO_EXCEPTION);
	~certificate_exception() throw();

	exception* clone() const;
	const char* name() const throw();
};


class VMIME_EXPORT certificate_verification_exception : public certificate_exception
{
public:

	certificate_verification_exception(const string& what, const exception& other = NO_EXCEPTION);
	~certificate_verification_exception() throw ();

	exception* clone() const;
	const char* name() const throw ();
};


class VMIME_EXPORT unsupported_certificate_type : public certificate_exception
{
public:

	unsupported_certificate_type(const string& type, const exception& other = NO_EXCEPTION);
	~unsupported_certificate_type() throw ();

	exception* clone() const;
	const char* name() const throw ();
};


#endif // VMIME_HAVE_TLS_SUPPORT



} // exceptions


} // vmime


#endif // VMIME_EXCEPTION_HPP_INCLUDED
