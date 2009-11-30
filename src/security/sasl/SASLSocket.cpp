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

#include "vmime/security/sasl/SASLSocket.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <algorithm>

#include <gsasl.h>


namespace vmime {
namespace security {
namespace sasl {



SASLSocket::SASLSocket(ref <SASLSession> sess, ref <net::socket> wrapped)
	: m_session(sess), m_wrapped(wrapped),
	  m_pendingBuffer(0), m_pendingPos(0), m_pendingLen(0)
{
}


SASLSocket::~SASLSocket()
{
	if (m_pendingBuffer)
		delete [] m_pendingBuffer;
}


void SASLSocket::connect(const string& address, const port_t port)
{
	m_wrapped->connect(address, port);
}


void SASLSocket::disconnect()
{
	m_wrapped->disconnect();
}


bool SASLSocket::isConnected() const
{
	return m_wrapped->isConnected();
}


SASLSocket::size_type SASLSocket::getBlockSize() const
{
	return m_wrapped->getBlockSize();
}


void SASLSocket::receive(string& buffer)
{
	const int n = receiveRaw(m_recvBuffer, sizeof(m_recvBuffer));

	buffer = string(m_recvBuffer, n);
}


SASLSocket::size_type SASLSocket::receiveRaw(char* buffer, const size_type count)
{
	if (m_pendingLen != 0)
	{
		const int copyLen =
			(count >= m_pendingLen ? m_pendingLen : count);

		std::copy(m_pendingBuffer + m_pendingPos,
		          m_pendingBuffer + m_pendingPos + copyLen,
		          buffer);

		m_pendingLen -= copyLen;
		m_pendingPos += copyLen;

		if (m_pendingLen == 0)
		{
			delete [] m_pendingBuffer;

			m_pendingBuffer = 0;
			m_pendingPos = 0;
			m_pendingLen = 0;
		}

		return copyLen;
	}

	const int n = m_wrapped->receiveRaw(buffer, count);

	byte_t* output = 0;
	int outputLen = 0;

	m_session->getMechanism()->decode
		(m_session, reinterpret_cast <const byte_t*>(buffer), n,
		 &output, &outputLen);

	// If we can not copy all decoded data into the output buffer, put
	// remaining data into a pending buffer for next calls to receive()
	if (outputLen > count)
	{
		std::copy(output, output + count, buffer);

		m_pendingBuffer = output;
		m_pendingLen = outputLen;
		m_pendingPos = count;

		return count;
	}
	else
	{
		std::copy(output, output + outputLen, buffer);

		delete [] output;

		return outputLen;
	}
}


void SASLSocket::send(const string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


void SASLSocket::sendRaw(const char* buffer, const size_type count)
{
	byte_t* output = 0;
	int outputLen = 0;

	m_session->getMechanism()->encode
		(m_session, reinterpret_cast <const byte_t*>(buffer), count,
		 &output, &outputLen);

	try
	{
		m_wrapped->sendRaw
			(reinterpret_cast <const char*>(output), outputLen);
	}
	catch (...)
	{
		delete [] output;
		throw;
	}

	delete [] output;
}


} // sasl
} // security
} // vmime

