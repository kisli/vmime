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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <errno.h>

#include "vmime/net/tls/gnutls/TLSSocket_GnuTLS.hpp"
#include "vmime/net/tls/gnutls/TLSSession_GnuTLS.hpp"

#include "vmime/platform.hpp"

#include "vmime/security/cert/X509Certificate.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <cstring>


namespace vmime {
namespace net {
namespace tls {


// static
shared_ptr <TLSSocket> TLSSocket::wrap(shared_ptr <TLSSession> session, shared_ptr <socket> sok)
{
	return make_shared <TLSSocket_GnuTLS>
		(dynamicCast <TLSSession_GnuTLS>(session), sok);
}


TLSSocket_GnuTLS::TLSSocket_GnuTLS(shared_ptr <TLSSession_GnuTLS> session, shared_ptr <socket> sok)
	: m_session(session), m_wrapped(sok), m_connected(false),
	  m_ex(NULL), m_status(0), m_errno(0)
{
	gnutls_transport_set_ptr(*m_session->m_gnutlsSession, this);

	gnutls_transport_set_push_function(*m_session->m_gnutlsSession, gnutlsPushFunc);
	gnutls_transport_set_pull_function(*m_session->m_gnutlsSession, gnutlsPullFunc);
	gnutls_transport_set_errno_function(*m_session->m_gnutlsSession, gnutlsErrnoFunc);
}


TLSSocket_GnuTLS::~TLSSocket_GnuTLS()
{
	if (m_ex)
	{
		delete m_ex;
		m_ex = NULL;
	}

	try
	{
		disconnect();
	}
	catch (...)
	{
		// Don't throw exception in destructor
	}
}


void TLSSocket_GnuTLS::connect(const string& address, const port_t port)
{
	try
	{
		m_wrapped->connect(address, port);

		handshake();
	}
	catch (...)
	{
		disconnect();
		throw;
	}
}


void TLSSocket_GnuTLS::disconnect()
{
	if (m_connected)
	{
		gnutls_bye(*m_session->m_gnutlsSession, GNUTLS_SHUT_RDWR);

		m_wrapped->disconnect();

		m_connected = false;
	}
}


bool TLSSocket_GnuTLS::isConnected() const
{
	return m_wrapped->isConnected() && m_connected;
}


size_t TLSSocket_GnuTLS::getBlockSize() const
{
	return 16384;  // 16 KB
}


const string TLSSocket_GnuTLS::getPeerName() const
{
	return m_wrapped->getPeerName();
}


const string TLSSocket_GnuTLS::getPeerAddress() const
{
	return m_wrapped->getPeerAddress();
}


shared_ptr <timeoutHandler> TLSSocket_GnuTLS::getTimeoutHandler()
{
	return m_wrapped->getTimeoutHandler();
}


void TLSSocket_GnuTLS::setTracer(shared_ptr <net::tracer> tracer)
{
	m_wrapped->setTracer(tracer);
}


shared_ptr <net::tracer> TLSSocket_GnuTLS::getTracer()
{
	return m_wrapped->getTracer();
}


bool TLSSocket_GnuTLS::waitForRead(const int msecs)
{
	return m_wrapped->waitForRead(msecs);
}


bool TLSSocket_GnuTLS::waitForWrite(const int msecs)
{
	return m_wrapped->waitForWrite(msecs);
}


void TLSSocket_GnuTLS::receive(string& buffer)
{
	const size_t size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = utility::stringUtils::makeStringFromBytes(m_buffer, size);
}


void TLSSocket_GnuTLS::send(const string& buffer)
{
	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void TLSSocket_GnuTLS::send(const char* str)
{
	sendRaw(reinterpret_cast <const byte_t*>(str), ::strlen(str));
}


size_t TLSSocket_GnuTLS::receiveRaw(byte_t* buffer, const size_t count)
{
	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	const ssize_t ret = gnutls_record_recv
		(*m_session->m_gnutlsSession,
		 buffer, static_cast <size_t>(count));

	if (m_ex)
		internalThrow();

	if (ret < 0)
	{
		if (ret == GNUTLS_E_AGAIN)
		{
			if (gnutls_record_get_direction(*m_session->m_gnutlsSession) == 0)
				m_status |= STATUS_WANT_READ;
			else
				m_status |= STATUS_WANT_WRITE;

			return 0;
		}

		TLSSession_GnuTLS::throwTLSException("gnutls_record_recv", static_cast <int>(ret));
	}

	return static_cast <size_t>(ret);
}


void TLSSocket_GnuTLS::sendRaw(const byte_t* buffer, const size_t count)
{
	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	for (size_t size = count ; size > 0 ; )
	{
		ssize_t ret = gnutls_record_send
			(*m_session->m_gnutlsSession,
			 buffer, static_cast <size_t>(size));

		if (m_ex)
			internalThrow();

		if (ret < 0)
		{
			if (ret == GNUTLS_E_AGAIN)
			{
				if (gnutls_record_get_direction(*m_session->m_gnutlsSession) == 0)
					m_wrapped->waitForRead();
				else
					m_wrapped->waitForWrite();

				continue;
			}

			TLSSession_GnuTLS::throwTLSException("gnutls_record_send", static_cast <int>(ret));
		}
		else
		{
			buffer += ret;
			size -= ret;
		}
	}
}


size_t TLSSocket_GnuTLS::sendRawNonBlocking(const byte_t* buffer, const size_t count)
{
	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	ssize_t ret = gnutls_record_send
		(*m_session->m_gnutlsSession,
		 buffer, static_cast <size_t>(count));

	if (m_ex)
		internalThrow();

	if (ret < 0)
	{
		if (ret == GNUTLS_E_AGAIN)
		{
			if (gnutls_record_get_direction(*m_session->m_gnutlsSession) == 0)
				m_status |= STATUS_WANT_READ;
			else
				m_status |= STATUS_WANT_WRITE;

			return 0;
		}

		TLSSession_GnuTLS::throwTLSException("gnutls_record_send", static_cast <int>(ret));
	}

	return static_cast <size_t>(ret);
}


unsigned int TLSSocket_GnuTLS::getStatus() const
{
	return m_status | m_wrapped->getStatus();
}


void TLSSocket_GnuTLS::handshake()
{
	shared_ptr <timeoutHandler> toHandler = m_wrapped->getTimeoutHandler();

	if (toHandler)
		toHandler->resetTimeOut();

	if (getTracer())
		getTracer()->traceSend("Beginning SSL/TLS handshake");

	// Start handshaking process
	try
	{
		while (true)
		{
			const int ret = gnutls_handshake(*m_session->m_gnutlsSession);

			if (m_ex)
				internalThrow();

			if (ret < 0)
			{
				if (ret == GNUTLS_E_AGAIN)
				{
					if (gnutls_record_get_direction(*m_session->m_gnutlsSession) == 0)
						m_wrapped->waitForRead();
					else
						m_wrapped->waitForWrite();
				}
				else if (ret == GNUTLS_E_INTERRUPTED)
				{
					// Non-fatal error
				}
				else
				{
					TLSSession_GnuTLS::throwTLSException("gnutls_handshake", ret);
				}
			}
			else
			{
				// Successful handshake
				break;
			}
		}
	}
	catch (...)
	{
		throw;
	}

	// Verify server's certificate(s)
	shared_ptr <security::cert::certificateChain> certs = getPeerCertificates();

	if (certs == NULL)
		throw exceptions::tls_exception("No peer certificate.");

	m_session->getCertificateVerifier()->verify(certs, getPeerName());

	m_connected = true;
}


int TLSSocket_GnuTLS::gnutlsErrnoFunc(gnutls_transport_ptr trspt)
{
	TLSSocket_GnuTLS* sok = reinterpret_cast <TLSSocket_GnuTLS*>(trspt);
	return sok->m_errno;
}


ssize_t TLSSocket_GnuTLS::gnutlsPushFunc
	(gnutls_transport_ptr trspt, const void* data, size_t len)
{
	TLSSocket_GnuTLS* sok = reinterpret_cast <TLSSocket_GnuTLS*>(trspt);

	try
	{
		const ssize_t ret = static_cast <ssize_t>
			(sok->m_wrapped->sendRawNonBlocking
				(reinterpret_cast <const byte_t*>(data), len));

		if (ret == 0)
		{
			gnutls_transport_set_errno(*sok->m_session->m_gnutlsSession, EAGAIN);
			sok->m_errno = EAGAIN;
			return -1;
		}

		return ret;
	}
	catch (exception& e)
	{
		// Workaround for non-portable behaviour when throwing C++ exceptions
		// from C functions (GNU TLS)
		sok->m_ex = e.clone();
		return -1;
	}
}


ssize_t TLSSocket_GnuTLS::gnutlsPullFunc
	(gnutls_transport_ptr trspt, void* data, size_t len)
{
	TLSSocket_GnuTLS* sok = reinterpret_cast <TLSSocket_GnuTLS*>(trspt);

	try
	{
		const ssize_t n = static_cast <ssize_t>
			(sok->m_wrapped->receiveRaw
				(reinterpret_cast <byte_t*>(data), len));

		if (n == 0)
		{
			gnutls_transport_set_errno(*sok->m_session->m_gnutlsSession, EAGAIN);
			sok->m_errno = EAGAIN;
			return -1;
		}

		return n;
	}
	catch (exception& e)
	{
		// Workaround for non-portable behaviour when throwing C++ exceptions
		// from C functions (GNU TLS)
		sok->m_ex = e.clone();
		return -1;
	}
}


shared_ptr <security::cert::certificateChain> TLSSocket_GnuTLS::getPeerCertificates()
{
	if (getTracer())
		getTracer()->traceSend("Getting peer certificates");

	unsigned int certCount = 0;
	const gnutls_datum* rawData = gnutls_certificate_get_peers
		(*m_session->m_gnutlsSession, &certCount);

	if (rawData == NULL)
		return null;

	// Try X.509
	gnutls_x509_crt* x509Certs = new gnutls_x509_crt[certCount];

	for (unsigned int i = 0; i < certCount; ++i)
	{
		gnutls_x509_crt_init(x509Certs + i);

		int res = gnutls_x509_crt_import(x509Certs[i], rawData + i,
			GNUTLS_X509_FMT_DER);

		if (res < 0)
		{
			// XXX more fine-grained error reporting?
			delete [] x509Certs;
			return null;
		}
	}

	{
		std::vector <shared_ptr <security::cert::certificate> > certs;
		bool error = false;

		for (unsigned int i = 0 ; i < certCount ; ++i)
		{
			size_t dataSize = 0;

			gnutls_x509_crt_export(x509Certs[i],
				GNUTLS_X509_FMT_DER, NULL, &dataSize);

			std::vector <byte_t> data(dataSize);

			gnutls_x509_crt_export(x509Certs[i],
				GNUTLS_X509_FMT_DER, &data[0], &dataSize);

			shared_ptr <security::cert::X509Certificate> cert =
				security::cert::X509Certificate::import(&data[0], dataSize);

			if (cert != NULL)
				certs.push_back(cert);
			else
				error = true;

			gnutls_x509_crt_deinit(x509Certs[i]);
		}

		delete [] x509Certs;

		if (error)
			return null;

		return make_shared <security::cert::certificateChain>(certs);
	}

	delete [] x509Certs;

	return null;
}


// Following is a workaround for C++ exceptions to pass correctly between
// C and C++ calls.
//
// gnutls_record_recv() calls TLSSocket::gnutlsPullFunc, and exceptions
// thrown by the socket can not be caught.

#ifndef VMIME_BUILDING_DOC

class TLSSocket_DeleteExWrapper : public object
{
public:

	TLSSocket_DeleteExWrapper(exception* ex) : m_ex(ex) { }
	~TLSSocket_DeleteExWrapper() { delete m_ex; }

private:

	exception* m_ex;
};

#endif // VMIME_BUILDING_DOC


void TLSSocket_GnuTLS::internalThrow()
{
	static std::vector <shared_ptr <TLSSocket_DeleteExWrapper> > exToDelete;

	if (m_ex)
	{
		// Reset the current exception pointer to prevent the same
		// exception from being thrown again later
		exception* ex = m_ex;
		m_ex = NULL;

		// To avoid memory leaks
		exToDelete.push_back(make_shared <TLSSocket_DeleteExWrapper>(ex));

		throw *ex;
	}
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
