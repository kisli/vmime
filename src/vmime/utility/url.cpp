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

#include "vmime/utility/url.hpp"

#include "vmime/parserHelpers.hpp"
#include "vmime/utility/urlUtils.hpp"
#include "vmime/exception.hpp"

#include <sstream>


namespace vmime {
namespace utility {


// Unspecified port
const port_t url::UNSPECIFIED_PORT = static_cast <port_t>(-1);

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

	m_params = u.m_params;

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
	oss.imbue(std::locale::classic());

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


	if (!m_params.empty())
	{
		if (m_path.empty())
			oss << "/";

		oss << "?";

		for (std::map <string, string>::const_iterator it = m_params.begin() ;
		     it != m_params.end() ; ++it)
		{
			if (it != m_params.begin())
				oss << "&";

			oss << urlUtils::encode((*it).first);
			oss << "=";
			oss << urlUtils::encode((*it).second);
		}
	}

	return (oss.str());
}


void url::parse(const string& str)
{
	// Protocol
	const size_t protoEnd = str.find("://");
	if (protoEnd == string::npos) throw exceptions::malformed_url("No protocol separator");

	const string proto =
		utility::stringUtils::toLower(string(str.begin(), str.begin() + protoEnd));

	// Username/password
	size_t slashPos = str.find('/', protoEnd + 3);
	if (slashPos == string::npos) slashPos = str.length();

	size_t atPos = str.rfind('@', slashPos);
	string hostPart;

	string username;
	string password;

	if (proto == PROTOCOL_FILE)
	{
		// No user name, password and host part.
		slashPos = protoEnd + 3;
	}
	else
	{
		if (atPos != string::npos && atPos < slashPos)
		{
			const string userPart(str.begin() + protoEnd + 3, str.begin() + atPos);
			const size_t colonPos = userPart.find(':');

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
			hostPart = string(str.begin() + protoEnd + 3, str.begin() + slashPos);
		}
	}

	// Host/port
	const size_t colonPos = hostPart.find(':');

	string host;
	string port;

	if (colonPos == string::npos)
	{
		host = utility::stringUtils::trim(hostPart);
	}
	else
	{
		host = utility::stringUtils::trim(string(hostPart.begin(), hostPart.begin() + colonPos));
		port = utility::stringUtils::trim(string(hostPart.begin() + colonPos + 1, hostPart.end()));
	}

	// Path
	string path = utility::stringUtils::trim(string(str.begin() + slashPos, str.end()));
	string params;

	size_t paramSep = path.find_first_of('?');

	if (paramSep != string::npos)
	{
		params = string(path.begin() + paramSep + 1, path.end());
		path.erase(path.begin() + paramSep, path.end());
	}

	if (path == "/")
		path.clear();

	// Some sanity check
	if (proto.empty())
		throw exceptions::malformed_url("No protocol specified");
	else if (host.empty())
	{
		// Accept empty host (eg. "file:///home/vincent/mydoc")
		if (proto != PROTOCOL_FILE)
			throw exceptions::malformed_url("No host specified");
	}

	bool onlyDigit = true;

	for (string::const_iterator it = port.begin() ;
	     onlyDigit && it != port.end() ; ++it)
	{
		onlyDigit = parserHelpers::isDigit(*it);
	}

	if (!onlyDigit)
		throw exceptions::malformed_url("Port can only contain digits");

	std::istringstream iss(port);
	port_t portNum = 0;

	iss >> portNum;

	if (portNum == 0)
		portNum = UNSPECIFIED_PORT;

	// Extract parameters
	m_params.clear();

	if (!params.empty())
	{
		size_t pos = 0;

		do
		{
			const size_t start = pos;

			pos = params.find_first_of('&', pos);

			const size_t equal = params.find_first_of('=', start);
			const size_t end =
				(pos == string::npos ? params.length() : pos);

			string name;
			string value;

			if (equal == string::npos || equal > pos) // no value
			{
				name = string(params.begin() + start, params.begin() + end);
				value = name;
			}
			else
			{
				name = string(params.begin() + start, params.begin() + equal);
				value = string(params.begin() + equal + 1, params.begin() + end);
			}

			name = urlUtils::decode(name);
			value = urlUtils::decode(value);

			m_params[name] = value;

			if (pos != string::npos)
				++pos;
		}
		while (pos != string::npos);
	}

	// Now, save URL parts
	m_protocol = proto;

	m_username = urlUtils::decode(username);
	m_password = urlUtils::decode(password);

	m_host = urlUtils::decode(host);
	m_port = portNum;

	m_path = urlUtils::decode(path);
}


const string& url::getProtocol() const
{
	return (m_protocol);
}


void url::setProtocol(const string& protocol)
{
	m_protocol = protocol;
}


const string& url::getUsername() const
{
	return (m_username);
}


void url::setUsername(const string& username)
{
	m_username = username;
}


const string& url::getPassword() const
{
	return (m_password);
}


void url::setPassword(const string& password)
{
	m_password = password;
}


const string& url::getHost() const
{
	return (m_host);
}


void url::setHost(const string& host)
{
	m_host = host;
}


port_t url::getPort() const
{
	return (m_port);
}


void url::setPort(const port_t port)
{
	m_port = port;
}


const string& url::getPath() const
{
	return (m_path);
}


void url::setPath(const string& path)
{
	m_path = path;
}


const std::map <string, string>& url::getParams() const
{
	return (m_params);
}


std::map <string, string>& url::getParams()
{
	return (m_params);
}


} // utility
} // vmime
