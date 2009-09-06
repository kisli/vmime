//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_UTILITY_URL_HPP_INCLUDED
#define VMIME_UTILITY_URL_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/base.hpp"
#include "vmime/propertySet.hpp"


namespace vmime {
namespace utility {


/** This class represents a Uniform Resource Locator (a pointer
  * to a "resource" on the World Wide Web).
  */

class url
{
public:

	/** Means "port not specified" (use default port). */
	static const port_t UNSPECIFIED_PORT;

	/** Standard name for FILE protocol (local file-system). */
	static const string PROTOCOL_FILE;

	/** Standard name for HTTP protocol. */
	static const string PROTOCOL_HTTP;

	/** Standard name for FTP protocol. */
	static const string PROTOCOL_FTP;


	/** Construct an URL from a string (parse the URL components).
	  *
	  * @param s full URL string (eg. http://www.vmime.org:80/download.html)
	  * @throw exceptions::malformed_url if URL is malformed
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


	/** Return the protocol of the URL (eg: "http").
	  *
	  * @return protocol of the URL
	  */
	const string& getProtocol() const;

	/** Set the protocol of the URL.
	  *
	  * @param protocol new protocol (eg: "http")
	  */
	void setProtocol(const string& protocol);

	/** Return the username specified in the URL
	  * or empty if not specified.
	  *
	  * @return user name
	  */
	const string& getUsername() const;

	/** Set the username of the URL.
	  *
	  * @param username user name
	  */
	void setUsername(const string& username);

	/** Return the password specified in the URL
	  * or empty if not specified.
	  *
	  * @return user password
	  */
	const string& getPassword() const;

	/** Set the password of the URL.
	  *
	  * @param password user password
	  */
	void setPassword(const string& password);

	/** Return the host name of the URL (server name or IP address).
	  *
	  * @return host name
	  */
	const string& getHost() const;

	/** Set the host name of the URL.
	  *
	  * @param host server name or IP address
	  */
	void setHost(const string& host);

	/** Return the port of the URL, or url::UNSPECIFIED_PORT if
	  * the default port if used.
	  *
	  * @return server port
	  */
	port_t getPort() const;

	/** Set the port of the URL.
	  *
	  * @param port server port or url::UNSPECIFIED_PORT to
	  * use the default port of the protocol
	  */
	void setPort(const port_t port);

	/** Return the path portion of the URL,
	  * or empty if not specified.
	  *
	  * @return path
	  */
	const string& getPath() const;

	/** Set the part portion of the URL.
	  *
	  * @param path path
	  */
	void setPath(const string& path);

	/** Return the parameters of the URL (read-only).
	  *
	  * @return parameters
	  */
	const propertySet& getParams() const;

	/** Return the parameters of the URL.
	  *
	  * @return parameters
	  */
	propertySet& getParams();

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

	propertySet m_params;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_URL_HPP_INCLUDED
