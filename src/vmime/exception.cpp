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

#include "vmime/exception.hpp"


namespace vmime {


//
// exception
//

const exception exception::NO_EXCEPTION;


exception::exception()
	: m_what(""), m_other(NULL)
{
}


exception::exception(const string& what, const exception& other)
	: m_what(what), m_other(&other != &NO_EXCEPTION ? other.clone() : NULL)
{
}


exception::exception(const exception& e)
	: std::exception(), m_what(e.what()), m_other(e.m_other == NULL ? NULL : e.m_other->clone())
{
}


exception::~exception() throw()
{
	delete (m_other);
}


const char* exception::what() const throw()
{
	return (m_what.c_str());
}


const char* exception::what() throw()
{
	return (m_what.c_str());
}


const exception* exception::other() const throw()
{
	return (m_other);
}


const char* exception::name() const throw()
{
	return "exception";
}


exception* exception::clone() const
{
	return new exception(*this);
}



namespace exceptions
{


//
// bad_field_value_type
//

bad_field_value_type::~bad_field_value_type() throw() {}
bad_field_value_type::bad_field_value_type(const string& fieldName, const exception& other)
	: exception("Bad value type for field '" + fieldName + "'.", other) {}

exception* bad_field_value_type::clone() const { return new bad_field_value_type(*this); }
const char* bad_field_value_type::name() const throw() { return "bad_field_value_type"; }



//
// charset_conv_error
//

charset_conv_error::~charset_conv_error() throw() {}
charset_conv_error::charset_conv_error(const string& what, const exception& other)
	: exception(what.empty() ? "Charset conversion error." : what, other) {}

exception* charset_conv_error::clone() const { return new charset_conv_error(*this); }
const char* charset_conv_error::name() const throw() { return "charset_conv_error"; }


//
// no_encoder_available
//

no_encoder_available::~no_encoder_available() throw() {}
no_encoder_available::no_encoder_available(const string& name, const exception& other)
	: exception("No encoder available: '" + name + "'.", other) {}

exception* no_encoder_available::clone() const { return new no_encoder_available(*this); }
const char* no_encoder_available::name() const throw() { return "no_encoder_available"; }


//
// no_digest_algorithm_available
//

no_digest_algorithm_available::~no_digest_algorithm_available() throw() {}
no_digest_algorithm_available::no_digest_algorithm_available(const string& name, const exception& other)
	: exception("No algorithm available: '" + name + "'.", other) {}

exception* no_digest_algorithm_available::clone() const { return new no_digest_algorithm_available(*this); }
const char* no_digest_algorithm_available::name() const throw() { return "no_digest_algorithm_available"; }


//
// no_such_field
//

no_such_field::~no_such_field() throw() {}
no_such_field::no_such_field(const exception& other)
	: exception("Field not found.", other) {}

exception* no_such_field::clone() const { return new no_such_field(*this); }
const char* no_such_field::name() const throw() { return "no_such_field"; }


//
// no_such_part
//

no_such_part::~no_such_part() throw() {}
no_such_part::no_such_part(const exception& other)
	: exception("Part not found.", other) {}

exception* no_such_part::clone() const { return new no_such_part(*this); }
const char* no_such_part::name() const throw() { return "no_such_part"; }


//
// no_such_message_id
//

no_such_message_id::~no_such_message_id() throw() {}
no_such_message_id::no_such_message_id(const exception& other)
	: exception("Message-Id not found.", other) {}

exception* no_such_message_id::clone() const { return new no_such_message_id(*this); }
const char* no_such_message_id::name() const throw() { return "no_such_message_id"; }


//
// open_file_error
//

open_file_error::~open_file_error() throw() {}
open_file_error::open_file_error(const exception& other)
	: exception("Error opening file.", other) {}

exception* open_file_error::clone() const { return new open_file_error(*this); }
const char* open_file_error::name() const throw() { return "open_file_error"; }


//
// no_factory_available
//

no_factory_available::~no_factory_available() throw() {}
no_factory_available::no_factory_available(const exception& other)
	: exception("No factory available.", other) {}

exception* no_factory_available::clone() const { return new no_factory_available(*this); }
const char* no_factory_available::name() const throw() { return "no_factory_available"; }


//
// no_platform_handler
//

no_platform_handler::~no_platform_handler() throw() {}
no_platform_handler::no_platform_handler(const exception& other)
	: exception("No platform handler installed.", other) {}

exception* no_platform_handler::clone() const { return new no_platform_handler(*this); }
const char* no_platform_handler::name() const throw() { return "no_platform_handler"; }


//
// no_expeditor
//

no_expeditor::~no_expeditor() throw() {}
no_expeditor::no_expeditor(const exception& other)
	: exception("No expeditor specified.", other) {}

exception* no_expeditor::clone() const { return new no_expeditor(*this); }
const char* no_expeditor::name() const throw() { return "no_expeditor"; }


//
// no_recipient
//

no_recipient::~no_recipient() throw() {}
no_recipient::no_recipient(const exception& other)
	: exception("No recipient specified.", other) {}

exception* no_recipient::clone() const { return new no_recipient(*this); }
const char* no_recipient::name() const throw() { return "no_recipient"; }


//
// no_such_property
//

no_such_property::~no_such_property() throw() {}
no_such_property::no_such_property(const string& name, const exception& other)
	: exception(string("No such property: '") + name + string("'."), other) { }

exception* no_such_property::clone() const { return new no_such_property(*this); }
const char* no_such_property::name() const throw() { return "no_such_property"; }


//
// invalid_property_type
//

invalid_property_type::~invalid_property_type() throw() {}
invalid_property_type::invalid_property_type(const exception& other)
	: exception("Invalid property type.", other) {}

exception* invalid_property_type::clone() const { return new invalid_property_type(*this); }
const char* invalid_property_type::name() const throw() { return "invalid_property_type"; }


//
// invalid_argument
//

invalid_argument::~invalid_argument() throw() {}
invalid_argument::invalid_argument(const exception& other)
	: exception("Invalid argument.", other) {}

exception* invalid_argument::clone() const { return new invalid_argument(*this); }
const char* invalid_argument::name() const throw() { return "invalid_argument"; }


//
// system_error
//

system_error::~system_error() throw() { }
system_error::system_error(const string& what, const exception& other)
	: exception(what, other) {}

exception* system_error::clone() const { return new system_error(*this); }
const char* system_error::name() const throw() { return "system_error"; }


//
// malformed_url
//

malformed_url::~malformed_url() throw() {}
malformed_url::malformed_url(const string& error, const exception& other)
	: exception("Malformed URL: " + error + ".", other) {}

exception* malformed_url::clone() const { return new malformed_url(*this); }
const char* malformed_url::name() const throw() { return "malformed_url"; }



#if VMIME_HAVE_MESSAGING_FEATURES


//
// net_exception
//

net_exception::~net_exception() throw() {}
net_exception::net_exception(const string& what, const exception& other)
	: exception(what, other) {}

exception* net_exception::clone() const { return new net_exception(*this); }
const char* net_exception::name() const throw() { return "net_exception"; }


//
// socket_exception
//

socket_exception::~socket_exception() throw() {}
socket_exception::socket_exception(const string& what, const exception& other)
	: net_exception(what.empty()
		? "Socket error." : what, other) {}

exception* socket_exception::clone() const { return new socket_exception(*this); }
const char* socket_exception::name() const throw() { return "socket_exception"; }


//
// socket_not_connected_exception
//

socket_not_connected_exception::~socket_not_connected_exception() throw() {}
socket_not_connected_exception::socket_not_connected_exception(const string& what, const exception& other)
	: socket_exception(what.empty()
		? "Socket is not connected." : what, other) {}

exception* socket_not_connected_exception::clone() const { return new socket_not_connected_exception(*this); }
const char* socket_not_connected_exception::name() const throw() { return "socket_not_connected_exception"; }


//
// connection_error
//

connection_error::~connection_error() throw() {}
connection_error::connection_error(const string& what, const exception& other)
	: socket_exception(what.empty()
		? "Connection error." : what, other) {}

exception* connection_error::clone() const { return new connection_error(*this); }
const char* connection_error::name() const throw() { return "connection_error"; }


//
// connection_greeting_error
//

connection_greeting_error::~connection_greeting_error() throw() {}
connection_greeting_error::connection_greeting_error(const string& response, const exception& other)
	: net_exception("Greeting error.", other), m_response(response) {}

const string& connection_greeting_error::response() const { return (m_response); }

exception* connection_greeting_error::clone() const { return new connection_greeting_error(*this); }
const char* connection_greeting_error::name() const throw() { return "connection_greeting_error"; }


//
// authentication_error
//

authentication_error::~authentication_error() throw() {}
authentication_error::authentication_error(const string& response, const exception& other)
	: net_exception("Authentication error.", other), m_response(response) {}

const string& authentication_error::response() const { return (m_response); }

exception* authentication_error::clone() const { return new authentication_error(*this); }
const char* authentication_error::name() const throw() { return "authentication_error"; }


//
// unsupported_option
//

unsupported_option::~unsupported_option() throw() {}
unsupported_option::unsupported_option(const exception& other)
	: net_exception("Unsupported option.", other) {}

exception* unsupported_option::clone() const { return new unsupported_option(*this); }
const char* unsupported_option::name() const throw() { return "unsupported_option"; }


//
// illegal_state
//

illegal_state::~illegal_state() throw() {}
illegal_state::illegal_state(const string& state, const exception& other)
	: net_exception("Illegal state to accomplish the operation: '" + state + "'.", other) {}

exception* illegal_state::clone() const { return new illegal_state(*this); }
const char* illegal_state::name() const throw() { return "illegal_state"; }


//
// folder_not_found
//

folder_not_found::~folder_not_found() throw() {}
folder_not_found::folder_not_found(const exception& other)
	: net_exception("Folder not found.", other) {}

exception* folder_not_found::clone() const { return new folder_not_found(*this); }
const char* folder_not_found::name() const throw() { return "folder_not_found"; }


//
// folder_already_open
//

folder_already_open::~folder_already_open() throw() {}
folder_already_open::folder_already_open(const exception& other)
	: net_exception("Folder is already open in the same session.", other) {}

exception* folder_already_open::clone() const { return new folder_already_open(*this); }
const char* folder_already_open::name() const throw() { return "folder_already_open"; }


//
// message_not_found
//

message_not_found::~message_not_found() throw() {}
message_not_found::message_not_found(const exception& other)
	: net_exception("Message not found.", other) {}

exception* message_not_found::clone() const { return new message_not_found(*this); }
const char* message_not_found::name() const throw() { return "message_not_found"; }


//
// operation_not_supported
//

operation_not_supported::~operation_not_supported() throw() {}
operation_not_supported::operation_not_supported(const exception& other)
	: net_exception("Operation not supported.", other) {}

exception* operation_not_supported::clone() const { return new operation_not_supported(*this); }
const char* operation_not_supported::name() const throw() { return "operation_not_supported"; }


//
// operation_timed_out
//

operation_timed_out::~operation_timed_out() throw() {}
operation_timed_out::operation_timed_out(const exception& other)
	: net_exception("Operation timed out.", other) {}

exception* operation_timed_out::clone() const { return new operation_timed_out(*this); }
const char* operation_timed_out::name() const throw() { return "operation_timed_out"; }


//
// operation_cancelled
//

operation_cancelled::~operation_cancelled() throw() {}
operation_cancelled::operation_cancelled(const exception& other)
	: net_exception("Operation cancelled by the user.", other) {}

exception* operation_cancelled::clone() const { return new operation_cancelled(*this); }
const char* operation_cancelled::name() const throw() { return "operation_cancelled"; }


//
// unfetched_object
//

unfetched_object::~unfetched_object() throw() {}
unfetched_object::unfetched_object(const exception& other)
	: net_exception("Object not fetched.", other) {}

exception* unfetched_object::clone() const { return new unfetched_object(*this); }
const char* unfetched_object::name() const throw() { return "unfetched_object"; }


//
// not_connected
//

not_connected::~not_connected() throw() {}
not_connected::not_connected(const exception& other)
	: net_exception("Not connected to a service.", other) {}

exception* not_connected::clone() const { return new not_connected(*this); }
const char* not_connected::name() const throw() { return "not_connected"; }


//
// already_connected
//

already_connected::~already_connected() throw() {}
already_connected::already_connected(const exception& other)
	: net_exception("Already connected to a service. Disconnect and retry.", other) {}

exception* already_connected::clone() const { return new already_connected(*this); }
const char* already_connected::name() const throw() { return "already_connected"; }


//
// illegal_operation
//

illegal_operation::~illegal_operation() throw() {}
illegal_operation::illegal_operation(const string& msg, const exception& other)
	: net_exception(msg.empty()
		? "Illegal operation."
		: "Illegal operation: " + msg + ".",
		other
	) {}

exception* illegal_operation::clone() const { return new illegal_operation(*this); }
const char* illegal_operation::name() const throw() { return "illegal_operation"; }


//
// command_error
//

command_error::~command_error() throw() {}
command_error::command_error(const string& command, const string& response,
                             const string& desc, const exception& other)
	: net_exception(desc.empty()
		? "Error while executing command '" + command + "'."
		: "Error while executing command '" + command + "': " + desc + ".",
		other
	),
	m_command(command), m_response(response) {}

const string& command_error::command() const { return (m_command); }

const string& command_error::response() const { return (m_response); }

exception* command_error::clone() const { return new command_error(*this); }
const char* command_error::name() const throw() { return "command_error"; }


//
// invalid_response
//

invalid_response::~invalid_response() throw() {}
invalid_response::invalid_response(const string& command, const string& response, const exception& other)
	: net_exception(command.empty()
		? "Received invalid response."
		: "Received invalid response for command '" + command + "'.",
		other
	),
	m_command(command), m_response(response) {}

const string& invalid_response::command() const { return (m_command); }

const string& invalid_response::response() const { return (m_response); }

exception* invalid_response::clone() const { return new invalid_response(*this); }
const char* invalid_response::name() const throw() { return "invalid_response"; }


//
// partial_fetch_not_supported
//

partial_fetch_not_supported::~partial_fetch_not_supported() throw() {}
partial_fetch_not_supported::partial_fetch_not_supported(const exception& other)
	: net_exception("Partial fetch not supported.", other) {}

exception* partial_fetch_not_supported::clone() const { return new partial_fetch_not_supported(*this); }
const char* partial_fetch_not_supported::name() const throw() { return "partial_fetch_not_supported"; }


//
// invalid_folder_name
//

invalid_folder_name::~invalid_folder_name() throw() {}
invalid_folder_name::invalid_folder_name(const string& error, const exception& other)
	: net_exception(error.empty()
		? "Invalid folder name: " + error + "."
		: "Invalid folder name.",
		other) {}

exception* invalid_folder_name::clone() const { return new invalid_folder_name(*this); }
const char* invalid_folder_name::name() const throw() { return "invalid_folder_name"; }


#endif // VMIME_HAVE_MESSAGING_FEATURES


#if VMIME_HAVE_FILESYSTEM_FEATURES


//
// filesystem_exception
//

filesystem_exception::~filesystem_exception() throw() {}
filesystem_exception::filesystem_exception(const string& what, const utility::path& path, const exception& other)
	: exception(what, other), m_path(path) {}

const utility::path& filesystem_exception::path() const { return (m_path); }

exception* filesystem_exception::clone() const { return new filesystem_exception(*this); }
const char* filesystem_exception::name() const throw() { return "filesystem_exception"; }


//
// not_a_directory
//

not_a_directory::~not_a_directory() throw() {}
not_a_directory::not_a_directory(const utility::path& path, const exception& other)
	: filesystem_exception("Operation failed: this is not a directory.", path, other) {}

exception* not_a_directory::clone() const { return new not_a_directory(*this); }
const char* not_a_directory::name() const throw() { return "not_a_directory"; }


//
// file_not_found
//

file_not_found::~file_not_found() throw() {}
file_not_found::file_not_found(const utility::path& path, const exception& other)
	: filesystem_exception("File not found.", path, other) {}

exception* file_not_found::clone() const { return new file_not_found(*this); }
const char* file_not_found::name() const throw() { return "file_not_found"; }


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


//
// authentication_exception
//

authentication_exception::~authentication_exception() throw() {}
authentication_exception::authentication_exception(const string& what, const exception& other)
	: exception(what, other) {}

exception* authentication_exception::clone() const { return new authentication_exception(*this); }
const char* authentication_exception::name() const throw() { return "authentication_exception"; }


//
// no_auth_information
//

no_auth_information::~no_auth_information() throw() {}
no_auth_information::no_auth_information(const exception& other)
	: authentication_exception("Information cannot be provided.", other) {}

exception* no_auth_information::clone() const { return new no_auth_information(*this); }
const char* no_auth_information::name() const throw() { return "no_auth_information"; }


#if VMIME_HAVE_SASL_SUPPORT


//
// sasl_exception
//

sasl_exception::~sasl_exception() throw() {}
sasl_exception::sasl_exception(const string& what, const exception& other)
	: authentication_exception(what, other) {}

exception* sasl_exception::clone() const { return new sasl_exception(*this); }
const char* sasl_exception::name() const throw() { return "sasl_exception"; }


//
// no_such_mechanism
//

no_such_mechanism::~no_such_mechanism() throw() {}
no_such_mechanism::no_such_mechanism(const string& name, const exception& other)
	: sasl_exception("No such SASL mechanism: '" + name + "'.", other) {}

exception* no_such_mechanism::clone() const { return new no_such_mechanism(*this); }
const char* no_such_mechanism::name() const throw() { return "no_such_mechanism"; }


#endif // VMIME_HAVE_SASL_SUPPORT


#if VMIME_HAVE_TLS_SUPPORT


//
// tls_exception
//

tls_exception::~tls_exception() throw() {}
tls_exception::tls_exception(const string& what, const exception& other)
	: exception(what, other) {}

exception* tls_exception::clone() const { return new tls_exception(*this); }
const char* tls_exception::name() const throw() { return "tls_exception"; }


//
// certificate_exception
//

certificate_exception::~certificate_exception() throw() {}
certificate_exception::certificate_exception(const string& what, const exception& other)
	: tls_exception(what, other) {}

exception* certificate_exception::clone() const { return new certificate_exception(*this); }
const char* certificate_exception::name() const throw() { return "certificate_exception"; }


//
// certificate_verification_exception
//

certificate_verification_exception::~certificate_verification_exception() throw() {}
certificate_verification_exception::certificate_verification_exception(const string& what, const exception& other)
	: certificate_exception(what, other) {}

exception* certificate_verification_exception::clone() const { return new certificate_verification_exception(*this); }
const char* certificate_verification_exception::name() const throw() { return "certificate_verification_exception"; }


//
// unsupported_certificate_type
//

unsupported_certificate_type::~unsupported_certificate_type() throw() {}
unsupported_certificate_type::unsupported_certificate_type(const string& type, const exception& other)
	: certificate_exception("Unsupported certificate type: '" + type + "'", other) {}

exception* unsupported_certificate_type::clone() const { return new unsupported_certificate_type(*this); }
const char* unsupported_certificate_type::name() const throw() { return "unsupported_certificate_type"; }


#endif // VMIME_HAVE_TLS_SUPPORT


} // exceptions


} // vmime

