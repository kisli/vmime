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

#include "url.hpp"

#include "parserHelpers.hpp"
#include "urlUtils.hpp"
#include "exception.hpp"

#include <sstream>


namespace vmime {
namespace messaging {


// Known protocols
const string url::PROTOCOL_FILE = "file";
const string url::PROTOCOL_HTTP = "http";
const string url::PROTOCOL_FTP = "ftp";



url::url(const string& s)
{
	parse(s);
}


url::url(const url& u)
{
	operator=(u);
}


url::url(const string& protocol, const string& host, const port_t port,
	const string& path, const string& username, const string& password)
	: m_protocol(protocol), m_username(username), m_password(password),
	  m_host(host), m_port(port), m_path(path)
{
}


url& url::operator=(const url& u)
{
	m_protocol = u.m_protocol;

	m_username = u.m_username;
	m_password = u.m_password;

	m_host = u.m_host;
	m_port = u.m_port;

	m_path = u.m_path;

	return (*this);
}


url& url::operator=(const string& s)
{
	parse(s);

	return (*this);
}


url::operator string() const
{
	return build();
}


const string url::build() const
{
	std::ostringstream oss;

	oss << m_protocol << "://";

	if (!m_username.empty())
	{
		oss << urlUtils::encode(m_username);

		if (!m_password.empty())
		{
			oss << ":";
			oss << urlUtils::encode(m_password);
		}

		oss << "@";
	}

	oss << urlUtils::encode(m_host);

	if (m_port != UNSPECIFIED_PORT)
	{
		oss << ":";
		oss << m_port;
	}

	if (!m_path.empty())
	{
		oss << "/";
		oss << urlUtils::encode(m_path);
	}

	return (oss.str());
}


void url::parse(const string& str)
{
	// Protocol
	const string::size_type protoEnd = str.find("://");
	if (protoEnd == string::npos) throw exceptions::malformed_url("No protocol separator");

	const string proto =
		toLower(string(str.begin(), str.begin() + protoEnd));

	// Username/password
	string::size_type slashPos = str.find('/', protoEnd + 3);
	if (slashPos == string::npos) slashPos = str.length();

	string::size_type atPos = str.find('@', protoEnd + 3);
	string hostPart;

	string username;
	string password;

	if (atPos != string::npos && atPos < slashPos)
	{
		const string userPart(str.begin() + protoEnd, str.begin() + atPos);
		const string::size_type colonPos = userPart.find(':');

		if (colonPos == string::npos)
		{
			username = userPart;
		}
		else
		{
			username = string(userPart.begin(), userPart.begin() + colonPos);
			password = string(userPart.begin() + colonPos + 1, userPart.end());
		}

		hostPart = string(str.begin() + atPos + 1, str.begin() + slashPos);
	}
	else
	{
		hostPart = string(str.begin() + protoEnd, str.begin() + slashPos);
	}

	// Host/port
	const string::size_type colonPos = hostPart.find(':');

	string host;
	string port;

	if (colonPos == string::npos)
	{
		host = hostPart;
	}
	else
	{
		host = string(hostPart.begin(), hostPart.begin() + colonPos);
		port = string(hostPart.begin() + colonPos + 1, hostPart.end());
	}

	// Path
	string path(str.begin() + slashPos, str.end());

	if (path == "/")
		path.clear();

	// Some sanity check
	if (proto.empty())
		throw exceptions::malformed_url("No protocol specified");
	else if (host.empty() && path.empty())  // Accept empty host (eg. "file:///home/vincent/mydoc")
		throw exceptions::malformed_url("No host specified");

	bool onlyDigit = true;

	for (string::const_iterator it = port.begin() ;
	     onlyDigit && it != port.end() ; ++it)
	{
		onlyDigit = isdigit(*it);
	}

	if (!onlyDigit)
		throw exceptions::malformed_url("Port can only contain digits");

	std::istringstream iss(port);
	port_t portNum = 0;

	iss >> portNum;

	// Now, save URL parts
	m_protocol = proto;

	m_username = urlUtils::decode(username);
	m_password = urlUtils::decode(password);

	m_host = urlUtils::decode(host);
	m_port = portNum;

	m_path = urlUtils::decode(path);
}


} // messaging
} // vmime
