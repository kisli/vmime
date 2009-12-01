//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "testUtils.hpp"



// testSocket

void testSocket::connect(const vmime::string& address, const vmime::port_t port)
{
	m_address = address;
	m_port = port;
	m_connected = true;

	onConnected();
}


void testSocket::disconnect()
{
	m_address.clear();
	m_port = 0;
	m_connected = false;
}


bool testSocket::isConnected() const
{
	return m_connected;
}


testSocket::size_type testSocket::getBlockSize() const
{
	return 16384;
}


void testSocket::receive(vmime::string& buffer)
{
	buffer = m_inBuffer;
	m_inBuffer.clear();
}


void testSocket::send(const vmime::string& buffer)
{
	m_outBuffer += buffer;

	onDataReceived();
}


int testSocket::receiveRaw(char* buffer, const int count)
{
	const int n = std::min(count, static_cast <int>(m_inBuffer.size()));

	std::copy(m_inBuffer.begin(), m_inBuffer.begin() + n, buffer);
	m_inBuffer.erase(m_inBuffer.begin(), m_inBuffer.begin() + n);

	return  n;
}


void testSocket::sendRaw(const char* buffer, const int count)
{
	send(vmime::string(buffer, count));
}


void testSocket::localSend(const vmime::string& buffer)
{
	m_inBuffer += buffer;
}


void testSocket::localReceive(vmime::string& buffer)
{
	buffer = m_outBuffer;
	m_outBuffer.clear();
}


void testSocket::onDataReceived()
{
	// Override
}


void testSocket::onConnected()
{
	// Override
}


// lineBasedTestSocket

void lineBasedTestSocket::onDataReceived()
{
	vmime::string chunk;
	localReceive(chunk);

	m_buffer += chunk;

	vmime::string::size_type eol;

	while ((eol = m_buffer.find('\n')) != vmime::string::npos)
	{
		vmime::string line(std::string(m_buffer.begin(), m_buffer.begin() + eol));

		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.end() - 1, line.end());

		m_lines.push_back(line);
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + eol + 1);
	}

	while (!m_lines.empty())
		processCommand();
}


const vmime::string lineBasedTestSocket::getNextLine()
{
	const vmime::string line = m_lines.front();
	m_lines.erase(m_lines.begin(), m_lines.begin() + 1);
	return line;
}

bool lineBasedTestSocket::haveMoreLines() const
{
	return !m_lines.empty();
}


// testTimeoutHandler

testTimeoutHandler::testTimeoutHandler(const unsigned int delay)
	: m_delay(delay), m_start(0)
{
}


bool testTimeoutHandler::isTimeOut()
{
	return (vmime::platform::getHandler()->getUnixTime() - m_start) >= m_delay;
}


void testTimeoutHandler::resetTimeOut()
{
	m_start = vmime::platform::getHandler()->getUnixTime();
}


bool testTimeoutHandler::handleTimeOut()
{
	return false;
}


// testTimeoutHandlerFactory : public vmime::net::timeoutHandlerFactory

vmime::ref <vmime::net::timeoutHandler> testTimeoutHandlerFactory::create()
{
	return vmime::create <testTimeoutHandler>();
}



// Exception helper
std::ostream& operator<<(std::ostream& os, const vmime::exception& e)
{
	os << "* vmime::exceptions::" << e.name() << std::endl;
	os << "    what = " << e.what() << std::endl;

	// More information for special exceptions
	if (dynamic_cast <const vmime::exceptions::command_error*>(&e))
	{
		const vmime::exceptions::command_error& cee =
			dynamic_cast <const vmime::exceptions::command_error&>(e);

		os << "    command = " << cee.command() << std::endl;
		os << "    response = " << cee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::invalid_response*>(&e))
	{
		const vmime::exceptions::invalid_response& ir =
			dynamic_cast <const vmime::exceptions::invalid_response&>(e);

		os << "    response = " << ir.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::connection_greeting_error*>(&e))
	{
		const vmime::exceptions::connection_greeting_error& cgee =
			dynamic_cast <const vmime::exceptions::connection_greeting_error&>(e);

		os << "    response = " << cgee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::authentication_error*>(&e))
	{
		const vmime::exceptions::authentication_error& aee =
			dynamic_cast <const vmime::exceptions::authentication_error&>(e);

		os << "    response = " << aee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::filesystem_exception*>(&e))
	{
		const vmime::exceptions::filesystem_exception& fse =
			dynamic_cast <const vmime::exceptions::filesystem_exception&>(e);

		os << "    path = " << vmime::platform::getHandler()->
			getFileSystemFactory()->pathToString(fse.path()) << std::endl;
	}

	if (e.other() != NULL)
		os << *e.other();

	return os;
}


