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

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "vmime/net/tls/TLSSocket.hpp"
#include "vmime/net/tls/TLSSession.hpp"

#include "vmime/platform.hpp"

#include "vmime/security/cert/X509Certificate.hpp"


namespace vmime {
namespace net {
namespace tls {


TLSSocket::TLSSocket(ref <TLSSession> session, ref <socket> sok)
	: m_session(session), m_wrapped(sok), m_connected(false),
	  m_handshaking(false), m_ex(NULL)
{
	gnutls_transport_set_ptr(*m_session->m_gnutlsSession, this);

	gnutls_transport_set_push_function(*m_session->m_gnutlsSession, gnutlsPushFunc);
	gnutls_transport_set_pull_function(*m_session->m_gnutlsSession, gnutlsPullFunc);
}


TLSSocket::~TLSSocket()
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


void TLSSocket::connect(const string& address, const port_t port)
{
	m_wrapped->connect(address, port);

	handshake(NULL);

	m_connected = true;
}


void TLSSocket::disconnect()
{
	if (m_connected)
	{
		gnutls_bye(*m_session->m_gnutlsSession, GNUTLS_SHUT_RDWR);

		m_wrapped->disconnect();

		m_connected = false;
	}
}


bool TLSSocket::isConnected() const
{
	return m_wrapped->isConnected() && m_connected;
}


TLSSocket::size_type TLSSocket::getBlockSize() const
{
	return 16384;  // 16 KB
}


void TLSSocket::receive(string& buffer)
{
	const int size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = vmime::string(m_buffer, size);
}


void TLSSocket::send(const string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


TLSSocket::size_type TLSSocket::receiveRaw(char* buffer, const size_type count)
{
	const ssize_t ret = gnutls_record_recv
		(*m_session->m_gnutlsSession,
		 buffer, static_cast <size_t>(count));

	if (m_ex)
		internalThrow();

	if (ret < 0)
	{
		if (ret == GNUTLS_E_AGAIN)
			return 0;

		TLSSession::throwTLSException("gnutls_record_recv", ret);
	}

	return static_cast <int>(ret);
}


void TLSSocket::sendRaw(const char* buffer, const size_type count)
{
	gnutls_record_send
		(*m_session->m_gnutlsSession,
		 buffer, static_cast <size_t>(count));

	if (m_ex)
		internalThrow();
}


void TLSSocket::handshake(ref <timeoutHandler> toHandler)
{
	if (toHandler)
		toHandler->resetTimeOut();

	// Start handshaking process
	m_handshaking = true;
	m_toHandler = toHandler;

	try
	{
		while (true)
		{
			const int ret = gnutls_handshake(*m_session->m_gnutlsSession);

			if (m_ex)
				internalThrow();

			if (ret < 0)
			{
				if (ret == GNUTLS_E_AGAIN ||
				    ret == GNUTLS_E_INTERRUPTED)
				{
					// Non-fatal error
					platform::getHandler()->wait();
				}
				else
				{
					TLSSession::throwTLSException("gnutls_handshake", ret);
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
		m_handshaking = false;
		m_toHandler = NULL;

		throw;
	}

	m_handshaking = false;
	m_toHandler = NULL;

	// Verify server's certificate(s)
	ref <security::cert::certificateChain> certs = getPeerCertificates();

	if (certs == NULL)
		throw exceptions::tls_exception("No peer certificate.");

	m_session->getCertificateVerifier()->verify(certs);

	m_connected = true;
}


ssize_t TLSSocket::gnutlsPushFunc
	(gnutls_transport_ptr trspt, const void* data, size_t len)
{
	TLSSocket* sok = reinterpret_cast <TLSSocket*>(trspt);

	try
	{
		sok->m_wrapped->sendRaw
			(reinterpret_cast <const char*>(data), static_cast <int>(len));
	}
	catch (exception& e)
	{
		// Workaround for bad behaviour when throwing C++ exceptions
		// from C functions (GNU TLS)
		sok->m_ex = e.clone();
		return -1;
	}

	return len;
}


ssize_t TLSSocket::gnutlsPullFunc
	(gnutls_transport_ptr trspt, void* data, size_t len)
{
	TLSSocket* sok = reinterpret_cast <TLSSocket*>(trspt);

	try
	{
		// Workaround for cross-platform asynchronous handshaking:
		// gnutls_handshake() only returns GNUTLS_E_AGAIN if recv()
		// returns -1 and errno is set to EGAIN...
		if (sok->m_handshaking)
		{
			while (true)
			{
				const ssize_t ret = static_cast <ssize_t>
					(sok->m_wrapped->receiveRaw
						(reinterpret_cast <char*>(data),
						 static_cast <int>(len)));

				if (ret == 0)
				{
					// No data available yet
					platform::getHandler()->wait();
				}
				else
				{
					return ret;
				}

				// Check whether the time-out delay is elapsed
				if (sok->m_toHandler && sok->m_toHandler->isTimeOut())
				{
					if (!sok->m_toHandler->handleTimeOut())
						throw exceptions::operation_timed_out();

					sok->m_toHandler->resetTimeOut();
				}
			}
		}
		else
		{
			const ssize_t n = static_cast <ssize_t>
				(sok->m_wrapped->receiveRaw
					(reinterpret_cast <char*>(data),
					 static_cast <int>(len)));

			if (n == 0)
				return GNUTLS_E_AGAIN;  // This seems like a hack, really...

			return n;
		}
	}
	catch (exception& e)
	{
		// Workaround for bad behaviour when throwing C++ exceptions
		// from C functions (GNU TLS)
		sok->m_ex = e.clone();
		return -1;
	}
}


ref <security::cert::certificateChain> TLSSocket::getPeerCertificates() const
{
	unsigned int certCount = 0;
	const gnutls_datum* rawData = gnutls_certificate_get_peers
		(*m_session->m_gnutlsSession, &certCount);

	if (rawData == NULL)
		return NULL;

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
			return NULL;
		}
	}

	{
		std::vector <ref <security::cert::certificate> > certs;
		bool error = false;

		for (unsigned int i = 0 ; i < certCount ; ++i)
		{
			size_t dataSize = 0;

			gnutls_x509_crt_export(x509Certs[i],
				GNUTLS_X509_FMT_DER, NULL, &dataSize);

			std::vector <byte_t> data(dataSize);

			gnutls_x509_crt_export(x509Certs[i],
				GNUTLS_X509_FMT_DER, &data[0], &dataSize);

			ref <security::cert::X509Certificate> cert =
				security::cert::X509Certificate::import(&data[0], dataSize);

			if (cert != NULL)
				certs.push_back(cert);
			else
				error = true;

			gnutls_x509_crt_deinit(x509Certs[i]);
		}

		delete [] x509Certs;

		if (error)
			return NULL;

		return vmime::create <security::cert::certificateChain>(certs);
	}

	delete [] x509Certs;

	return NULL;
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


void TLSSocket::internalThrow()
{
	static std::vector <ref <TLSSocket_DeleteExWrapper> > exToDelete;

	if (m_ex)
	{
		// Reset the current exception pointer to prevent the same
		// exception from being thrown again later
		exception* ex = m_ex;
		m_ex = NULL;

		// To avoid memory leaks
		exToDelete.push_back(vmime::create <TLSSocket_DeleteExWrapper>(ex));

		throw *ex;
	}
}


} // tls
} // net
} // vmime

