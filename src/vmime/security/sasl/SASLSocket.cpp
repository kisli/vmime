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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/security/sasl/SASLSocket.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/exception.hpp"

#include <algorithm>
#include <cstring>

#include <gsasl.h>


namespace vmime {
namespace security {
namespace sasl {



SASLSocket::SASLSocket(shared_ptr <SASLSession> sess, shared_ptr <net::socket> wrapped)
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


size_t SASLSocket::getBlockSize() const
{
	return m_wrapped->getBlockSize();
}


const string SASLSocket::getPeerName() const
{
	return m_wrapped->getPeerName();
}


const string SASLSocket::getPeerAddress() const
{
	return m_wrapped->getPeerAddress();
}


shared_ptr <net::timeoutHandler> SASLSocket::getTimeoutHandler()
{
	return m_wrapped->getTimeoutHandler();
}


void SASLSocket::setTracer(shared_ptr <net::tracer> tracer)
{
	m_wrapped->setTracer(tracer);
}


shared_ptr <net::tracer> SASLSocket::getTracer()
{
	return m_wrapped->getTracer();
}


bool SASLSocket::waitForRead(const int msecs)
{
	return m_wrapped->waitForRead(msecs);
}


bool SASLSocket::waitForWrite(const int msecs)
{
	return m_wrapped->waitForWrite(msecs);
}


void SASLSocket::receive(string& buffer)
{
	const size_t n = receiveRaw(m_recvBuffer, sizeof(m_recvBuffer));

	buffer = utility::stringUtils::makeStringFromBytes(m_recvBuffer, n);
}


size_t SASLSocket::receiveRaw(byte_t* buffer, const size_t count)
{
	if (m_pendingLen != 0)
	{
		const size_t copyLen =
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

	const size_t n = m_wrapped->receiveRaw(buffer, count);

	byte_t* output = 0;
	size_t outputLen = 0;

	m_session->getMechanism()->decode
		(m_session, buffer, n, &output, &outputLen);

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
	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void SASLSocket::send(const char* str)
{
	sendRaw(reinterpret_cast <const byte_t*>(str), strlen(str));
}


void SASLSocket::sendRaw(const byte_t* buffer, const size_t count)
{
	byte_t* output = 0;
	size_t outputLen = 0;

	m_session->getMechanism()->encode
		(m_session, buffer, count, &output, &outputLen);

	try
	{
		m_wrapped->sendRaw(output, outputLen);
	}
	catch (...)
	{
		delete [] output;
		throw;
	}

	delete [] output;
}


size_t SASLSocket::sendRawNonBlocking(const byte_t* buffer, const size_t count)
{
	byte_t* output = 0;
	size_t outputLen = 0;

	m_session->getMechanism()->encode
		(m_session, buffer, count, &output, &outputLen);

	size_t bytesSent = 0;

	try
	{
		bytesSent = m_wrapped->sendRawNonBlocking(output, outputLen);
	}
	catch (...)
	{
		delete [] output;
		throw;
	}

	delete [] output;

	return bytesSent;
}


unsigned int SASLSocket::getStatus() const
{
	return m_wrapped->getStatus();
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

