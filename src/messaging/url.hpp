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

#ifndef VMIME_MESSAGING_URL_HPP_INCLUDED
#define VMIME_MESSAGING_URL_HPP_INCLUDED


#include "../types.hpp"
#include "../base.hpp"


namespace vmime {
namespace messaging {


/** This class represents a Uniform Resource Locator (a pointer
  * to a "resource" on the World Wide Web).
  */

class url
{
public:

	/** Means "port not specified" (use default port).
	  */
	static const port_t UNSPECIFIED_PORT = static_cast <port_t>(-1);

	static const string PROTOCOL_FILE;
	static const string PROTOCOL_HTTP;
	static const string PROTOCOL_FTP;


	/** Construct an URL from a string (parse the URL components).
	  *
	  * @param s full URL string (eg. http://vmime.sourceforge.net:80/download.html
	  */
	url(const string& s);

	/** Construct an URL from another URL object.
	  *
	  * @param u other URL object
	  */
	url(const url& u);

	/** Construct an URL from the components.
	  *
	  * @param protocol protocol (eg. "http", "ftp"...)
	  * @param host host name (eg. "vmime.sourceforge.net", "123.45.67.89")
	  * @param port optional port number (eg. 80, 110 or UNSPECIFIED_PORT to mean "default")
	  * @param path optional full path (eg. "download.html")
	  * @param username optional user name
	  * @param password optional user password
	  */
	url(const string& protocol, const string& host, const port_t port = UNSPECIFIED_PORT,
		const string& path = "", const string& username = "", const string& password = "");


	const string& protocol() const { return (m_protocol); }
	string& protocol() { return (m_protocol); }

	const string& username() const { return (m_username); }
	string& username() { return (m_username); }

	const string& password() const { return (m_password); }
	string& password() { return (m_password); }

	const string& host() const { return (m_host); }
	string& host() { return (m_host); }

	const port_t port() const { return (m_port); }
	port_t& port() { return (m_port); }

	const string& path() const { return (m_path); }
	string& path() { return (m_path); }


	/** Build a string URL from this object.
	  */
	operator string() const;

	url& operator=(const url& u);
	url& operator=(const string& s);

private:

	const string build() const;
	void parse(const string& str);

	// Format:
	// "protocol://[username[:password]@]host[:port][/path]"

	string m_protocol;

	string m_username;
	string m_password;

	string m_host;

	port_t m_port;

	string m_path;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_URL_HPP_INCLUDED
