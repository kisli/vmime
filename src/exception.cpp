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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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


exception::~exception()
{
	delete (m_other);
}


const string exception::what() const throw()
{
	return (m_what);
}


const exception* exception::other() const
{
	return (m_other);
}


const string exception::name() const
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
// bad_field_type
//

bad_field_type::~bad_field_type() throw() {}
bad_field_type::bad_field_type(const exception& other)
	: exception("Bad field type.", other) {}

exception* bad_field_type::clone() const { return new bad_field_type(*this); }
const string bad_field_type::name() const { return "bad_field_type"; }



//
// charset_conv_error
//

charset_conv_error::~charset_conv_error() throw() {}
charset_conv_error::charset_conv_error(const exception& other)
	: exception("Charset conversion error.", other) {}

exception* charset_conv_error::clone() const { return new charset_conv_error(*this); }
const string charset_conv_error::name() const { return "charset_conv_error"; }


//
// no_encoder_available
//

no_encoder_available::~no_encoder_available() throw() {}
no_encoder_available::no_encoder_available(const exception& other)
	: exception("No encoder available.", other) {}

exception* no_encoder_available::clone() const { return new no_encoder_available(*this); }
const string no_encoder_available::name() const { return "no_encoder_available"; }


//
// no_such_parameter
//

no_such_parameter::~no_such_parameter() throw() {}
no_such_parameter::no_such_parameter(const string& name, const exception& other)
	: exception(string("Parameter not found: '") + name + string("'."), other) {}

exception* no_such_parameter::clone() const { return new no_such_parameter(*this); }
const string no_such_parameter::name() const { return "no_such_parameter"; }


//
// no_such_field
//

no_such_field::~no_such_field() throw() {}
no_such_field::no_such_field(const exception& other)
	: exception("Field not found.", other) {}

exception* no_such_field::clone() const { return new no_such_field(*this); }
const string no_such_field::name() const { return "no_such_field"; }


//
// no_such_part
//

no_such_part::~no_such_part() throw() {}
no_such_part::no_such_part(const exception& other)
	: exception("Part not found.", other) {}

exception* no_such_part::clone() const { return new no_such_part(*this); }
const string no_such_part::name() const { return "no_such_part"; }


//
// no_such_mailbox
//

no_such_mailbox::~no_such_mailbox() throw() {}
no_such_mailbox::no_such_mailbox(const exception& other)
	: exception("Mailbox not found.", other) {}

exception* no_such_mailbox::clone() const { return new no_such_mailbox(*this); }
const string no_such_mailbox::name() const { return "no_such_mailbox"; }


//
// no_such_message_id
//

no_such_message_id::~no_such_message_id() throw() {}
no_such_message_id::no_such_message_id(const exception& other)
	: exception("Message-Id not found.", other) {}

exception* no_such_message_id::clone() const { return new no_such_message_id(*this); }
const string no_such_message_id::name() const { return "no_such_message_id"; }


//
// no_such_address
//

no_such_address::~no_such_address() throw() {}
no_such_address::no_such_address(const exception& other)
	: exception("Address not found.", other) {}

exception* no_such_address::clone() const { return new no_such_address(*this); }
const string no_such_address::name() const { return "no_such_address"; }


//
// open_file_error
//

open_file_error::~open_file_error() throw() {}
open_file_error::open_file_error(const exception& other)
	: exception("Error opening file.", other) {}

exception* open_file_error::clone() const { return new open_file_error(*this); }
const string open_file_error::name() const { return "open_file_error"; }


//
// no_factory_available
//

no_factory_available::~no_factory_available() throw() {}
no_factory_available::no_factory_available(const exception& other)
	: exception("No factory available.", other) {}

exception* no_factory_available::clone() const { return new no_factory_available(*this); }
const string no_factory_available::name() const { return "no_factory_available"; }


//
// no_platform_dependant_handler
//

no_platform_dependant_handler::~no_platform_dependant_handler() throw() {}
no_platform_dependant_handler::no_platform_dependant_handler(const exception& other)
	: exception("No platform-dependant handler installed.", other) {}

exception* no_platform_dependant_handler::clone() const { return new no_platform_dependant_handler(*this); }
const string no_platform_dependant_handler::name() const { return "no_platform_dependant_handler"; }


//
// no_expeditor
//

no_expeditor::~no_expeditor() throw() {}
no_expeditor::no_expeditor(const exception& other)
	: exception("No expeditor specified.", other) {}

exception* no_expeditor::clone() const { return new no_expeditor(*this); }
const string no_expeditor::name() const { return "no_expeditor"; }


//
// no_recipient
//

no_recipient::~no_recipient() throw() {}
no_recipient::no_recipient(const exception& other)
	: exception("No recipient specified.", other) {}

exception* no_recipient::clone() const { return new no_recipient(*this); }
const string no_recipient::name() const { return "no_recipient"; }


//
// no_object_found
//

no_object_found::~no_object_found() throw() {}
no_object_found::no_object_found(const exception& other)
	: exception("No object found.", other) {}

exception* no_object_found::clone() const { return new no_object_found(*this); }
const string no_object_found::name() const { return "no_object_found"; }


//
// no_such_property
//

no_such_property::~no_such_property() throw() {}
no_such_property::no_such_property(const string& name, const exception& other)
	: exception(std::string("No such property: '") + name + string("'."), other) { }

exception* no_such_property::clone() const { return new no_such_property(*this); }
const string no_such_property::name() const { return "no_such_property"; }


//
// invalid_property_type
//

invalid_property_type::~invalid_property_type() throw() {}
invalid_property_type::invalid_property_type(const exception& other)
	: exception("Invalid property type.", other) {}

exception* invalid_property_type::clone() const { return new invalid_property_type(*this); }
const string invalid_property_type::name() const { return "invalid_property_type"; }


//
// invalid_argument
//

invalid_argument::~invalid_argument() throw() {}
invalid_argument::invalid_argument(const exception& other)
	: exception("Invalid argument.", other) {}

exception* invalid_argument::clone() const { return new invalid_argument(*this); }
const string invalid_argument::name() const { return "invalid_argument"; }



#if VMIME_HAVE_MESSAGING_FEATURES


//
// messaging_exception
//

messaging_exception::~messaging_exception() throw() {}
messaging_exception::messaging_exception(const string& what, const exception& other)
	: exception(what, other) {}

exception* messaging_exception::clone() const { return new messaging_exception(*this); }
const string messaging_exception::name() const { return "messaging_exception"; }


//
// connection_error
//

connection_error::~connection_error() throw() {}
connection_error::connection_error(const string& what, const exception& other)
	: messaging_exception(what.empty()
		? "Connection error."
		: "Connection error: '" + what + "'.", other) {}

exception* connection_error::clone() const { return new connection_error(*this); }
const string connection_error::name() const { return "connection_error"; }


//
// connection_greeting_error
//

connection_greeting_error::~connection_greeting_error() throw() {}
connection_greeting_error::connection_greeting_error(const string& response, const exception& other)
	: messaging_exception("Greeting error.", other), m_response(response) {}

const string& connection_greeting_error::response() const { return (m_response); }

exception* connection_greeting_error::clone() const { return new connection_greeting_error(*this); }
const string connection_greeting_error::name() const { return "connection_greeting_error"; }


//
// authentication_error
//

authentication_error::~authentication_error() throw() {}
authentication_error::authentication_error(const string& response, const exception& other)
	: messaging_exception("Authentication error.", other), m_response(response) {}

const string& authentication_error::response() const { return (m_response); }

exception* authentication_error::clone() const { return new authentication_error(*this); }
const string authentication_error::name() const { return "authentication_error"; }


//
// unsupported_option
//

unsupported_option::~unsupported_option() throw() {}
unsupported_option::unsupported_option(const exception& other)
	: messaging_exception("Unsupported option.", other) {}

exception* unsupported_option::clone() const { return new unsupported_option(*this); }
const string unsupported_option::name() const { return "unsupported_option"; }


//
// no_service_available
//

no_service_available::~no_service_available() throw() {}
no_service_available::no_service_available(const string& proto, const exception& other)
	: messaging_exception(proto.empty()
		? "No service available for this protocol."
		: "No service available for this protocol: '" + proto + "'.", other) {}

exception* no_service_available::clone() const { return new no_service_available(*this); }
const string no_service_available::name() const { return "no_service_available"; }


//
// illegal_state
//

illegal_state::~illegal_state() throw() {}
illegal_state::illegal_state(const string& state, const exception& other)
	: messaging_exception("Illegal state to accomplish the operation: '" + state + "'.", other) {}

exception* illegal_state::clone() const { return new illegal_state(*this); }
const string illegal_state::name() const { return "illegal_state"; }


//
// folder_not_found
//

folder_not_found::~folder_not_found() throw() {}
folder_not_found::folder_not_found(const exception& other)
	: messaging_exception("Folder not found.", other) {}

exception* folder_not_found::clone() const { return new folder_not_found(*this); }
const string folder_not_found::name() const { return "folder_not_found"; }


//
// message_not_found
//

message_not_found::~message_not_found() throw() {}
message_not_found::message_not_found(const exception& other)
	: messaging_exception("Message not found.", other) {}

exception* message_not_found::clone() const { return new message_not_found(*this); }
const string message_not_found::name() const { return "message_not_found"; }


//
// operation_not_supported
//

operation_not_supported::~operation_not_supported() throw() {}
operation_not_supported::operation_not_supported(const exception& other)
	: messaging_exception("Operation not supported.", other) {}

exception* operation_not_supported::clone() const { return new operation_not_supported(*this); }
const string operation_not_supported::name() const { return "operation_not_supported"; }


//
// operation_timed_out
//

operation_timed_out::~operation_timed_out() throw() {}
operation_timed_out::operation_timed_out(const exception& other)
	: messaging_exception("Operation timed out.", other) {}

exception* operation_timed_out::clone() const { return new operation_timed_out(*this); }
const string operation_timed_out::name() const { return "operation_timed_out"; }


//
// operation_cancelled
//

operation_cancelled::~operation_cancelled() throw() {}
operation_cancelled::operation_cancelled(const exception& other)
	: messaging_exception("Operation cancelled by the user.", other) {}

exception* operation_cancelled::clone() const { return new operation_cancelled(*this); }
const string operation_cancelled::name() const { return "operation_cancelled"; }


//
// unfetched_object
//

unfetched_object::~unfetched_object() throw() {}
unfetched_object::unfetched_object(const exception& other)
	: messaging_exception("Object not fetched.", other) {}

exception* unfetched_object::clone() const { return new unfetched_object(*this); }
const string unfetched_object::name() const { return "unfetched_object"; }


//
// not_connected
//

not_connected::~not_connected() throw() {}
not_connected::not_connected(const exception& other)
	: messaging_exception("Not connected to a service.", other) {}

exception* not_connected::clone() const { return new not_connected(*this); }
const string not_connected::name() const { return "not_connected"; }


//
// already_connected
//

already_connected::~already_connected() throw() {}
already_connected::already_connected(const exception& other)
	: messaging_exception("Already connected to a service. Disconnect and retry.", other) {}

exception* already_connected::clone() const { return new already_connected(*this); }
const string already_connected::name() const { return "already_connected"; }


//
// illegal_operation
//

illegal_operation::~illegal_operation() throw() {}
illegal_operation::illegal_operation(const string& msg, const exception& other)
	: messaging_exception(msg.empty()
		? "Illegal operation."
		: "Illegal operation: " + msg + ".",
		other
	) {}

exception* illegal_operation::clone() const { return new illegal_operation(*this); }
const string illegal_operation::name() const { return "illegal_operation"; }


//
// command_error
//

command_error::~command_error() throw() {}
command_error::command_error(const string& command, const string& response,
                             const string& desc, const exception& other)
	: messaging_exception(desc.empty()
		? "Error while executing command '" + command + "'."
		: "Error while executing command '" + command + "': " + desc + ".",
		other
	),
	m_command(command), m_response(response) {}

const string& command_error::command() const { return (m_command); }

const string& command_error::response() const { return (m_response); }

exception* command_error::clone() const { return new command_error(*this); }
const string command_error::name() const { return "command_error"; }


//
// invalid_response
//

invalid_response::~invalid_response() throw() {}
invalid_response::invalid_response(const string& command, const string& response, const exception& other)
	: messaging_exception(command.empty()
		? "Received invalid response."
		: "Received invalid response for command '" + command + "'.",
		other
	),
	m_command(command), m_response(response) {}

const string& invalid_response::command() const { return (m_command); }

const string& invalid_response::response() const { return (m_response); }

exception* invalid_response::clone() const { return new invalid_response(*this); }
const string invalid_response::name() const { return "invalid_response"; }


//
// partial_fetch_not_supported
//

partial_fetch_not_supported::~partial_fetch_not_supported() throw() {}
partial_fetch_not_supported::partial_fetch_not_supported(const exception& other)
	: messaging_exception("Partial fetch not supported.", other) {}

exception* partial_fetch_not_supported::clone() const { return new partial_fetch_not_supported(*this); }
const string partial_fetch_not_supported::name() const { return "partial_fetch_not_supported"; }


//
// malformed_url
//

malformed_url::~malformed_url() throw() {}
malformed_url::malformed_url(const string& error, const exception& other)
	: messaging_exception("Malformed URL: " + error + ".", other) {}

exception* malformed_url::clone() const { return new malformed_url(*this); }
const string malformed_url::name() const { return "malformed_url"; }


//
// invalid_folder_name
//

invalid_folder_name::~invalid_folder_name() throw() {}
invalid_folder_name::invalid_folder_name(const string& error, const exception& other)
	: messaging_exception(error.empty()
		? "Invalid folder name: " + error + "."
		: "Invalid folder name.",
		other) {}

exception* invalid_folder_name::clone() const { return new invalid_folder_name(*this); }
const string invalid_folder_name::name() const { return "invalid_folder_name"; }


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
const string filesystem_exception::name() const { return "filesystem_exception"; }


//
// not_a_directory
//

not_a_directory::~not_a_directory() throw() {}
not_a_directory::not_a_directory(const utility::path& path, const exception& other)
	: filesystem_exception("Operation failed: this is not a directory.", path, other) {}

exception* not_a_directory::clone() const { return new not_a_directory(*this); }
const string not_a_directory::name() const { return "not_a_directory"; }


//
// file_not_found
//

file_not_found::~file_not_found() throw() {}
file_not_found::file_not_found(const utility::path& path, const exception& other)
	: filesystem_exception("File not found.", path, other) {}

exception* file_not_found::clone() const { return new file_not_found(*this); }
const string file_not_found::name() const { return "file_not_found"; }


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


} // exceptions


} // vmime
