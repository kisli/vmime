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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include <openssl/ssl.h>
#include <openssl/err.h>

#include "vmime/net/tls/openssl/TLSSocket_OpenSSL.hpp"
#include "vmime/net/tls/openssl/TLSSession_OpenSSL.hpp"

#include "vmime/platform.hpp"

#include "vmime/security/cert/openssl/X509Certificate_OpenSSL.hpp"

#include <vector>


namespace vmime {
namespace net {
namespace tls {


// static
ref <TLSSocket> TLSSocket::wrap(ref <TLSSession> session, ref <socket> sok)
{
	return vmime::create <TLSSocket_OpenSSL>
		(session.dynamicCast <TLSSession_OpenSSL>(), sok);
}


TLSSocket_OpenSSL::TLSSocket_OpenSSL(ref <TLSSession_OpenSSL> session, ref <socket> sok)
	: m_session(session), m_wrapped(sok), m_connected(false), m_ssl(0)
{
}


TLSSocket_OpenSSL::~TLSSocket_OpenSSL()
{
	try
	{
		disconnect();

		if (m_ssl)
		{
			SSL_free(m_ssl);
			m_ssl = 0;
		}
	}
	catch (...)
	{
		// Don't throw in destructor
	}
}


void TLSSocket_OpenSSL::createSSLHandle()
{
	if (m_wrapped->isConnected())
	{
		static BIO_METHOD customBIOMethod;
		::memset(&customBIOMethod, 0, sizeof(customBIOMethod));

		customBIOMethod.type = 100 | BIO_TYPE_SOURCE_SINK;
		customBIOMethod.name = "vmime::socket glue";
		customBIOMethod.bwrite = bio_write;
		customBIOMethod.bread = bio_read;
		customBIOMethod.bputs = bio_puts;
		customBIOMethod.bgets = bio_gets;
		customBIOMethod.ctrl = bio_ctrl;
		customBIOMethod.create = bio_create;
		customBIOMethod.destroy = bio_destroy;

		BIO* sockBio = BIO_new(&customBIOMethod);
		sockBio->ptr = this;

		m_ssl = SSL_new(m_session->getContext());

		if (!m_ssl)
		{
			BIO_free(sockBio);
			throw exceptions::tls_exception("Cannot create SSL object");
		}

		SSL_set_bio(m_ssl, sockBio, sockBio);
		SSL_set_connect_state(m_ssl);
	}
	else
	{
		throw exceptions::tls_exception("Unconnected socket error");
	}
}


void TLSSocket_OpenSSL::connect(const string& address, const port_t port)
{
	m_wrapped->connect(address, port);

	createSSLHandle();

	handshake(NULL);

	m_connected = true;
}


void TLSSocket_OpenSSL::disconnect()
{
	if (m_connected)
	{
		if (m_ssl)
		{
			// Don't shut down the socket more than once.
			int shutdownState = SSL_get_shutdown(m_ssl);
			bool shutdownSent = (shutdownState & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN;

			if (!shutdownSent)
				SSL_shutdown(m_ssl);
		}

		m_wrapped->disconnect();
		m_connected = false;
	}
}


bool TLSSocket_OpenSSL::isConnected() const
{
	return m_wrapped->isConnected() && m_connected;
}


TLSSocket::size_type TLSSocket_OpenSSL::getBlockSize() const
{
	return 16384;  // 16 KB
}


void TLSSocket_OpenSSL::receive(string& buffer)
{
	const int size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = vmime::string(m_buffer, size);
}


void TLSSocket_OpenSSL::send(const string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


TLSSocket::size_type TLSSocket_OpenSSL::receiveRaw(char* buffer, const size_type count)
{
	int rc = SSL_read(m_ssl, buffer, count);
	handleError(rc);
	return rc;
}


void TLSSocket_OpenSSL::sendRaw(const char* buffer, const size_type count)
{
	int rc = SSL_write(m_ssl, buffer, count);
	handleError(rc);
}


void TLSSocket_OpenSSL::handshake(ref <timeoutHandler> toHandler)
{
	if (toHandler)
		toHandler->resetTimeOut();

	// Start handshaking process
	m_toHandler = toHandler;

	if (!m_ssl)
		createSSLHandle();

	try
	{
		// int ret = SSL_connect(m_ssl);
		int ret = SSL_do_handshake(m_ssl);
		handleError(ret);
	}
	catch (...)
	{
		SSL_free(m_ssl);
		m_ssl = 0;
		m_toHandler = NULL;
		throw;
	}

	m_toHandler = NULL;

	// Verify server's certificate(s)
	ref <security::cert::certificateChain> certs = getPeerCertificates();

	if (certs == NULL)
		throw exceptions::tls_exception("No peer certificate.");

	m_session->getCertificateVerifier()->verify(certs);

	m_connected = true;
}


ref <security::cert::certificateChain> TLSSocket_OpenSSL::getPeerCertificates() const
{
	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(m_ssl);

	if (chain == NULL)
		return NULL;

	int certCount = sk_X509_num(chain);

	if (certCount == 0)
		return NULL;

	bool error = false;
	std::vector <ref <security::cert::certificate> > certs;

	for (int i = 0; i < certCount && !error; i++)
	{
		ref <vmime::security::cert::X509Certificate> cert =
			vmime::security::cert::X509Certificate_OpenSSL::importInternal(sk_X509_value(chain, i));

		if (cert)
			certs.push_back(cert);
		else
			error = true;
	}

	if (error)
		return NULL;

	return vmime::create <security::cert::certificateChain>(certs);
}


void TLSSocket_OpenSSL::handleError(int rc)
{
	if (rc > 0) return;

	int  sslError   = SSL_get_error(m_ssl, rc);
	long lastError  = ERR_get_error();

	switch (sslError)
	{
	case SSL_ERROR_ZERO_RETURN:
		return;

	case SSL_ERROR_SYSCALL:
	{
		if (lastError == 0)
		{
			if (rc == 0)
				throw exceptions::tls_exception("SSL connection unexpectedly closed");
			else
			{
				vmime::string msg;
				std::ostringstream oss(msg);
				oss << "The BIO reported an error: %d" << rc;
				oss.flush();
				throw exceptions::tls_exception(oss.str());
			}
		}
		break;
	}

	//// Follwoing errors should not occur
	// With SSL_MODE_AUTO_RETRY these should not happen
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	// This happens only for BIOs of type BIO_s_connect() or BIO_s_accept()
	case SSL_ERROR_WANT_CONNECT:
	case SSL_ERROR_WANT_ACCEPT:
	// SSL_CTX_set_client_cert_cb related, not used
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_SSL:
	default:

		if (lastError == 0)
		{
			throw exceptions::tls_exception("Unexpected SSL IO error");
		}
		else
		{
			char buffer[256];
			ERR_error_string_n(lastError, buffer, sizeof(buffer));
			vmime::string msg(buffer);
			throw exceptions::tls_exception(msg);
		}

		break;
	}
}


// Implementation of custom BIO methods


// static
int TLSSocket_OpenSSL::bio_write(BIO* bio, const char* buf, int len)
{
	if (buf == NULL || len <= 0)
		return 0;

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(bio->ptr);

	sok->m_wrapped->sendRaw(buf, len);

	return len;
}


// static
int TLSSocket_OpenSSL::bio_read(BIO* bio, char* buf, int len)
{
	if (buf == NULL || len <= 0)
		return 0;

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(bio->ptr);

	const int n = sok->m_wrapped->receiveRaw(buf, len);

	if (n == 0)
		BIO_set_retry_read(bio);  // This seems like a hack, really...
	else
		BIO_clear_retry_flags(bio);

	return n;
}


// static
int TLSSocket_OpenSSL::bio_puts(BIO* bio, const char* str)
{
	return bio_write(bio, str, strlen(str));
}


// static
int TLSSocket_OpenSSL::bio_gets(BIO* /* bio */, char* /* buf */, int /* len */)
{
	return -1;
}


// static
long TLSSocket_OpenSSL::bio_ctrl(BIO* /* bio */, int cmd, long /* num */, void* /* ptr */)
{
	if (cmd == BIO_CTRL_FLUSH)
	{
		// OpenSSL library needs this
		return 1;
	}

	return 0;
}


// static
int TLSSocket_OpenSSL::bio_create(BIO* bio)
{
	bio->init = 1;
	bio->num = 0;
	bio->ptr = NULL;
	bio->flags = 0;

	return 1;
}


// static
int TLSSocket_OpenSSL::bio_destroy(BIO* bio)
{
	if (bio == NULL)
		return 0;

	bio->ptr = NULL;
	bio->init = 0;
	bio->flags = 0;

	return 1;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL
