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

	const std::vector <ref <const propertySet::property> > params
		= m_params.getPropertyList();

	if (!params.empty())
	{
		if (m_path.empty())
			oss << "/";

		oss << "?";

		for (unsigned int i = 0 ; i < params.size() ; ++i)
		{
			const ref <const propertySet::property> prop = params[i];

			if (i != 0)
				oss << "&";

			oss << urlUtils::encode(prop->getName());
			oss << "=";
			oss << urlUtils::encode(prop->getValue());
		}
	}

	return (oss.str());
}


void url::parse(const string& str)
{
	// Protocol
	const string::size_type protoEnd = str.find("://");
	if (protoEnd == string::npos) throw exceptions::malformed_url("No protocol separator");

	const string proto =
		utility::stringUtils::toLower(string(str.begin(), str.begin() + protoEnd));

	// Username/password
	string::size_type slashPos = str.find('/', protoEnd + 3);
	if (slashPos == string::npos) slashPos = str.length();

	string::size_type atPos = str.rfind('@', slashPos);
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
			hostPart = string(str.begin() + protoEnd + 3, str.begin() + slashPos);
		}
	}

	// Host/port
	const string::size_type colonPos = hostPart.find(':');

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

	string::size_type paramSep = path.find_first_of('?');

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
	m_params.removeAllProperties();

	if (!params.empty())
	{
		string::size_type pos = 0;

		do
		{
			const string::size_type start = pos;

			pos = params.find_first_of('&', pos);

			const string::size_type equal = params.find_first_of('=', start);
			const string::size_type end =
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

			m_params.setProperty(name, value);

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


const propertySet& url::getParams() const
{
	return (m_params);
}


propertySet& url::getParams()
{
	return (m_params);
}


} // utility
} // vmime
