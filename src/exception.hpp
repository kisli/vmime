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
protected:

	string m_what;

public:

	exception(const string& what) : m_what(what) { }
	virtual ~exception() { }

	const string what() const throw() { return (m_what); };
};


namespace exceptions
{


class bad_field_type : public vmime::exception
{
public:

	bad_field_type() : exception("Bad field type.") {}
	~bad_field_type() throw() {}
};


class charset_conv_error : public vmime::exception
{
public:

	charset_conv_error() : exception("Charset conversion error.") {}
	~charset_conv_error() throw() {}
};


class no_encoder_available : public vmime::exception
{
public:

	no_encoder_available() : exception("No encoder available.") {}
	~no_encoder_available() throw() {}
};


class no_such_parameter : public vmime::exception
{
public:

	no_such_parameter(const string& name) : exception
		(string("Parameter not found: '") + name + string("'.")) {}
	~no_such_parameter() throw() {}
};


class no_such_field : public vmime::exception
{
public:

	no_such_field() : exception("Field not found.") {}
	~no_such_field() throw() {}
};


class open_file_error : public vmime::exception
{
public:

	open_file_error() : exception("Error opening file.") {}
	~open_file_error() throw() {}
};


class no_factory_available : public vmime::exception
{
public:

	no_factory_available() : exception("No factory available.") {}
	~no_factory_available() throw() {}
};


class no_platform_dependant_handler : public vmime::exception
{
public:

	no_platform_dependant_handler() : exception("No platform-dependant handler installed.") {}
	~no_platform_dependant_handler() throw() {}
};


class no_expeditor : public vmime::exception
{
public:

	no_expeditor() : exception("No expeditor specified.") {}
	~no_expeditor() throw() {}
};


class no_recipient : public vmime::exception
{
public:

	no_recipient() : exception("No recipient specified.") {}
	~no_recipient() throw() {}
};


class no_object_found : public vmime::exception
{
public:

	no_object_found() : exception("No object found.") {}
	~no_object_found() throw() {}
};


// There is no property with that name in the set.

class no_such_property : public vmime::exception
{
public:

	no_such_property(const string& name) : exception
		(std::string("No such property: '") + name + string("'.")) { }
	~no_such_property() throw() {}
};


// Bad type specified when reading property.

class invalid_property_type : public vmime::exception
{
public:

	invalid_property_type() : exception("Invalid property type.") {}
	~invalid_property_type() throw() {}
};


// Bad argument was passed to the function.

class invalid_argument : public vmime::exception
{
public:

	invalid_argument() : exception("Invalid argument.") {}
	~invalid_argument() throw() {}
};



#if VMIME_HAVE_MESSAGING_FEATURES


/** Base class for exceptions thrown by the messaging module.
  */

class messaging_exception : public vmime::exception
{
public:

	messaging_exception(const string& what) : exception(what) {}
	~messaging_exception() throw() {}
};


/** Error while connecting to the server: this may be a DNS resolution error
  * or a connection error (for example, time-out while connecting).
  */

class connection_error : public messaging_exception
{
public:

	connection_error() : messaging_exception("Connection error.") {}
	~connection_error() throw() {}
};


/** Server did not initiated the connection correctly.
  */

class connection_greeting_error : public messaging_exception
{
public:

	connection_greeting_error(const string& response)
		: messaging_exception("Greeting error."), m_response(response) {}
	~connection_greeting_error() throw() {}

	const string& response() const { return (m_response); }

private:

	string m_response;
};


/** Error while giving credentials to the server (wrong username
  * or password, or wrong authentication method).
  */

class authentication_error : public messaging_exception
{
public:

	authentication_error(const string& response)
		: messaging_exception("Authentication error."), m_response(response) {}
	~authentication_error() throw() {}

	const string& response() const { return (m_response); }

private:

	string m_response;
};


/** Option not supported.
  */

class unsupported_option : public messaging_exception
{
public:

	unsupported_option() : messaging_exception("Unsupported option.") {}
	~unsupported_option() throw() {}
};


/** No service available for this protocol.
  */

class no_service_available : public messaging_exception
{
public:

	no_service_available() : messaging_exception("No service available for this protocol.") {}
	~no_service_available() throw() {}
};


/** The current state of the object does not permit to execute the
  * operation (for example, you try to close a folder which is not open).
  */

class illegal_state : public messaging_exception
{
public:

	illegal_state(const string& state)
		: messaging_exception("Illegal state to accomplish the operation: '" + state + "'.") {}
	~illegal_state() throw() {}
};


/** Folder not found (does not exist).
  */

class folder_not_found : public messaging_exception
{
public:

	folder_not_found() : messaging_exception("Folder not found.") {}
	~folder_not_found() throw() {}
};


/** Message not found (does not exist).
  */

class message_not_found : public messaging_exception
{
public:

	message_not_found() : messaging_exception("Message not found.") {}
	~message_not_found() throw() {}
};


/** Operation not supported by the underlying protocol.
  */

class operation_not_supported : public messaging_exception
{
public:

	operation_not_supported() : messaging_exception("Operation not supported.") {}
	~operation_not_supported() throw() {}
};


/** The operation timed out (time-out delay is elapsed).
  */

class operation_timed_out : public messaging_exception
{
public:

	operation_timed_out() : messaging_exception("Operation timed out.") {}
	~operation_timed_out() throw() {}
};


/** The operation has been cancelled.
  */

class operation_cancelled : public messaging_exception
{
public:

	operation_cancelled() : messaging_exception("Operation cancelled by the user.") {}
	~operation_cancelled() throw() {}
};


/** Must call fetchMessage() or fetchHeader() before accessing
  * the requested object.
  */

class unfetched_object : public messaging_exception
{
public:

	unfetched_object() : messaging_exception("Object not fetched.") {}
	~unfetched_object() throw() {}
};


/** The service is not currently connected.
  */

class not_connected : public messaging_exception
{
public:

	not_connected() : messaging_exception("Not connected to a service.") {}
	~not_connected() throw() {}
};


/** The service is already connected (must disconnect before).
  */

class already_connected : public messaging_exception
{
public:

	already_connected() : messaging_exception("Already connected to a service. Disconnect and retry.") {}
	~already_connected() throw() {}
};


/** Command error: operation failed (this is specific to the underlying protocol).
  */

class command_error : public messaging_exception
{
public:

	command_error(const string& command, const string& response, const string& desc = "")
		: messaging_exception(desc.empty()
			? "Error while executing command '" + command + "'."
			: "Error while executing command '" + command + "': " + desc + "."
		  ),
		  m_command(command), m_response(response) {}
	~command_error() throw() {}

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependant.
	  *
	  * @return command name (protocol-dependant)
	  */
	const string& command() const { return (m_command); }

	/** Return the invalid response line.
	  * The meaning is protocol-dependant.
	  *
	  * @return response line (protocol-dependant)
	  */
	const string& response() const { return (m_response); }

private:

	string m_command;
	string m_response;
};


/** The server returned an invalid response.
  */

class invalid_response : public messaging_exception
{
public:

	invalid_response(const string& command, const string& response)
		: messaging_exception(command.empty()
			? "Received invalid response."
			: "Received invalid response for command '" + command + "'."
		  ),
		  m_command(command), m_response(response) {}
	~invalid_response() throw() {}

	/** Return the name of the command which have thrown the exception.
	  * This is protocol-dependant.
	  *
	  * @return command name (protocol-dependant)
	  */
	const string& command() const { return (m_command); }

	/** Return the invalid response line.
	  * The meaning is protocol-dependant.
	  *
	  * @return response line (protocol-dependant)
	  */
	const string& response() const { return (m_response); }

private:

	string m_command;
	string m_response;
};


/** Partial fetch is not supported by the underlying protocol.
  */

class partial_fetch_not_supported : public messaging_exception
{
public:

	partial_fetch_not_supported() : messaging_exception("Partial fetch not supported.") {}
	~partial_fetch_not_supported() throw() {}
};


/** The URL is malformed.
  */

class malformed_url : public messaging_exception
{
public:

	malformed_url(const string& error) : messaging_exception("Malformed URL: " + error + ".") {}
	~malformed_url() throw() {}
};


/** Folder name is invalid.
  */

class invalid_folder_name : public messaging_exception
{
public:

	invalid_folder_name(const string& error) : messaging_exception("Invalid folder name: " + error + ".") {}
	~invalid_folder_name() throw() {}
};


#endif // VMIME_HAVE_MESSAGING_FEATURES


#if VMIME_HAVE_FILESYSTEM_FEATURES


/** Base class for exceptions thrown by the filesystem features.
  */

class filesystem_exception : public vmime::exception
{
public:

	filesystem_exception(const string& what, const utility::path& path) : exception(what), m_path(path) {}
	~filesystem_exception() throw() {}

	/** Return the full path of the file have thrown the exception.
	  *
	  * @return full path of the file/directory
	  */
	const utility::path& path() const { return (m_path); }

private:

	const utility::path m_path;
};


/** File is not a directory.
  */

class not_a_directory : public filesystem_exception
{
public:

	not_a_directory(const utility::path& path) : filesystem_exception("Operation failed: this is not a directory.", path) {}
	~not_a_directory() throw() {}
};


/** File not found.
  */

class file_not_found : public filesystem_exception
{
public:

	file_not_found(const utility::path& path) : filesystem_exception("File not found.", path) {}
	~file_not_found() throw() {}
};


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


} // exceptions


} // vmime


#endif // VMIME_EXCEPTION_HPP_INCLUDED
