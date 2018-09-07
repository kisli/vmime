//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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
#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/platform.hpp"

#include "vmime/security/cert/openssl/X509Certificate_OpenSSL.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <vector>
#include <cstring>


namespace vmime {
namespace net {
namespace tls {


static OpenSSLInitializer::autoInitializer openSSLInitializer;


#if OPENSSL_VERSION_NUMBER < 0x10100000L

// static
BIO_METHOD TLSSocket_OpenSSL::sm_customBIOMethod = {
	100 | BIO_TYPE_SOURCE_SINK,
	"vmime::socket glue",
	TLSSocket_OpenSSL::bio_write,
	TLSSocket_OpenSSL::bio_read,
	TLSSocket_OpenSSL::bio_puts,
	NULL,  // gets
	TLSSocket_OpenSSL::bio_ctrl,
	TLSSocket_OpenSSL::bio_create,
	TLSSocket_OpenSSL::bio_destroy,
	0
};

#define BIO_set_init(b, val) b->init = val
#define BIO_set_data(b, val) b->ptr = val
#define BIO_set_num(b, val) b->num = val
#define BIO_set_flags(b, val) b->flags = val
#define BIO_set_shutdown(b, val) b->shutdown = val
#define BIO_get_init(b) b->init
#define BIO_get_data(b) b->ptr
#define BIO_get_shutdown(b) b->shutdown

#else

#define BIO_set_num(b, val)

#endif



// static
shared_ptr <TLSSocket> TLSSocket::wrap(
	const shared_ptr <TLSSession>& session,
	const shared_ptr <socket>& sok
) {

	return make_shared <TLSSocket_OpenSSL>(dynamicCast <TLSSession_OpenSSL>(session), sok);
}


TLSSocket_OpenSSL::TLSSocket_OpenSSL(
	const shared_ptr <TLSSession_OpenSSL>& session,
	const shared_ptr <socket>& sok
)
	: m_session(session),
	  m_wrapped(sok),
	  m_connected(false),
	  m_ssl(0),
	  m_status(0),
	  m_ex() {

}


TLSSocket_OpenSSL::~TLSSocket_OpenSSL() {

	try {
		disconnect();
	} catch (...) {
		// Don't throw in destructor
	}
}


void TLSSocket_OpenSSL::createSSLHandle() {

	if (m_wrapped->isConnected()) {

#if OPENSSL_VERSION_NUMBER < 0x10100000L

		BIO* sockBio = BIO_new(&sm_customBIOMethod);
		sockBio->ptr = this;
		sockBio->init = 1;

#else

		BIO_METHOD* bioMeth = BIO_meth_new(BIO_TYPE_SOURCE_SINK | BIO_get_new_index(), "vmime::socket glue");

		if (!bioMeth) {
			BIO_meth_free(bioMeth);
			throw exceptions::tls_exception("BIO_meth_new() failed");
		}

		BIO_meth_set_write(bioMeth, TLSSocket_OpenSSL::bio_write);
		BIO_meth_set_read(bioMeth, TLSSocket_OpenSSL::bio_read);
		BIO_meth_set_puts(bioMeth, TLSSocket_OpenSSL::bio_puts);
		BIO_meth_set_ctrl(bioMeth, TLSSocket_OpenSSL::bio_ctrl);
		BIO_meth_set_create(bioMeth, TLSSocket_OpenSSL::bio_create);
		BIO_meth_set_destroy(bioMeth, TLSSocket_OpenSSL::bio_destroy);

		BIO* sockBio = BIO_new(bioMeth);
		BIO_set_data(sockBio, this);
		BIO_set_init(sockBio, 1);

#endif

		if (!sockBio) {
			throw exceptions::tls_exception("BIO_new() failed");
		}

		m_ssl = SSL_new(m_session->getContext());

		if (!m_ssl) {
			BIO_free(sockBio);
			throw exceptions::tls_exception("Cannot create SSL object");
		}

		SSL_set_bio(m_ssl, sockBio, sockBio);
		SSL_set_connect_state(m_ssl);
		SSL_set_mode(m_ssl, SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	} else {

		throw exceptions::tls_exception("Unconnected socket error");
	}
}


void TLSSocket_OpenSSL::connect(const string& address, const port_t port) {

	try {

		m_wrapped->connect(address, port);

		createSSLHandle();

		handshake();

	} catch (...) {

		disconnect();
		throw;
	}
}


void TLSSocket_OpenSSL::disconnect() {

	if (m_ssl) {

		// Don't shut down the socket more than once.
		int shutdownState = SSL_get_shutdown(m_ssl);
		bool shutdownSent = (shutdownState & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN;

		if (!shutdownSent) {
			SSL_shutdown(m_ssl);
		}

		SSL_free(m_ssl);
		m_ssl = 0;
	}

	if (m_connected) {
		m_connected = false;
		m_wrapped->disconnect();
	}
}


bool TLSSocket_OpenSSL::isConnected() const {

	return m_wrapped->isConnected() && m_connected;
}


size_t TLSSocket_OpenSSL::getBlockSize() const {

	return 16384;  // 16 KB
}


const string TLSSocket_OpenSSL::getPeerName() const {

	return m_wrapped->getPeerName();
}


const string TLSSocket_OpenSSL::getPeerAddress() const {

	return m_wrapped->getPeerAddress();
}


shared_ptr <timeoutHandler> TLSSocket_OpenSSL::getTimeoutHandler() {

	return m_wrapped->getTimeoutHandler();
}


void TLSSocket_OpenSSL::setTracer(const shared_ptr <net::tracer>& tracer) {

	m_wrapped->setTracer(tracer);
}


shared_ptr <net::tracer> TLSSocket_OpenSSL::getTracer() {

	return m_wrapped->getTracer();
}


bool TLSSocket_OpenSSL::waitForRead(const int msecs) {

	return m_wrapped->waitForRead(msecs);
}


bool TLSSocket_OpenSSL::waitForWrite(const int msecs) {

	return m_wrapped->waitForWrite(msecs);
}


void TLSSocket_OpenSSL::receive(string& buffer) {

	const size_t size = receiveRaw(m_buffer, sizeof(m_buffer));

	if (size != 0) {
		buffer = utility::stringUtils::makeStringFromBytes(m_buffer, size);
	} else {
		buffer.clear();
	}
}


void TLSSocket_OpenSSL::send(const string& buffer) {

	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void TLSSocket_OpenSSL::send(const char* str) {

	sendRaw(reinterpret_cast <const byte_t*>(str), ::strlen(str));
}


size_t TLSSocket_OpenSSL::receiveRaw(byte_t* buffer, const size_t count) {

	if (!m_ssl) {
		throw exceptions::socket_not_connected_exception();
	}

	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	ERR_clear_error();
	int rc = SSL_read(m_ssl, buffer, static_cast <int>(count));

	if (m_ex.get()) {
		internalThrow();
	}

	if (rc <= 0) {

		int error = SSL_get_error(m_ssl, rc);

		if (error == SSL_ERROR_WANT_WRITE) {
			m_status |= STATUS_WANT_WRITE;
			return 0;
		} else if (error == SSL_ERROR_WANT_READ) {
			m_status |= STATUS_WANT_READ;
			return 0;
		}

		handleError(rc);
	}

	return rc;
}


void TLSSocket_OpenSSL::sendRaw(const byte_t* buffer, const size_t count) {

	if (!m_ssl) {
		throw exceptions::socket_not_connected_exception();
	}

	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	for (size_t size = count ; size > 0 ; ) {

		ERR_clear_error();
		int rc = SSL_write(m_ssl, buffer, static_cast <int>(size));

		if (rc <= 0) {

			int error = SSL_get_error(m_ssl, rc);

			if (error == SSL_ERROR_WANT_READ) {
				m_wrapped->waitForRead();
				continue;
			} else if (error == SSL_ERROR_WANT_WRITE) {
				m_wrapped->waitForWrite();
				continue;
			}

			handleError(rc);

		} else {

			buffer += rc;
			size -= rc;
		}
	}
}


size_t TLSSocket_OpenSSL::sendRawNonBlocking(const byte_t* buffer, const size_t count) {

	if (!m_ssl) {
		throw exceptions::socket_not_connected_exception();
	}

	m_status &= ~(STATUS_WANT_WRITE | STATUS_WANT_READ);

	ERR_clear_error();
	int rc = SSL_write(m_ssl, buffer, static_cast <int>(count));

	if (m_ex.get()) {
		internalThrow();
	}

	if (rc <= 0) {

		int error = SSL_get_error(m_ssl, rc);

		if (error == SSL_ERROR_WANT_WRITE) {
			m_status |= STATUS_WANT_WRITE;
			return 0;
		} else if (error == SSL_ERROR_WANT_READ) {
			m_status |= STATUS_WANT_READ;
			return 0;
		}

		handleError(rc);
	}

	return rc;
}


void TLSSocket_OpenSSL::handshake() {

	shared_ptr <timeoutHandler> toHandler = m_wrapped->getTimeoutHandler();

	if (toHandler) {
		toHandler->resetTimeOut();
	}

	if (getTracer()) {
		getTracer()->traceSend("Beginning SSL/TLS handshake");
	}

	// Start handshaking process
	if (!m_ssl) {
		createSSLHandle();
	}

	try {

		int rc;

		ERR_clear_error();

		while ((rc = SSL_do_handshake(m_ssl)) <= 0) {

			const int err = SSL_get_error(m_ssl, rc);

			if (err == SSL_ERROR_WANT_READ) {
				m_wrapped->waitForRead();
			} else if (err == SSL_ERROR_WANT_WRITE) {
				m_wrapped->waitForWrite();
			} else {
				handleError(rc);
			}

			// Check whether the time-out delay is elapsed
			if (toHandler && toHandler->isTimeOut()) {

				if (!toHandler->handleTimeOut()) {
					throw exceptions::operation_timed_out();
				}

				toHandler->resetTimeOut();
			}

			ERR_clear_error();
		}

	} catch (...) {

		throw;
	}

	// Verify server's certificate(s)
	shared_ptr <security::cert::certificateChain> certs = getPeerCertificates();

	if (!certs) {
		throw exceptions::tls_exception("No peer certificate.");
	}

	m_session->getCertificateVerifier()->verify(certs, getPeerName());

	m_connected = true;
}


shared_ptr <security::cert::certificateChain> TLSSocket_OpenSSL::getPeerCertificates() {

	if (getTracer()) {
		getTracer()->traceSend("Getting peer certificates");
	}

	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(m_ssl);

	if (chain == NULL) {
		return null;
	}

	int certCount = sk_X509_num(chain);

	if (certCount == 0) {
		return null;
	}

	bool error = false;
	std::vector <shared_ptr <security::cert::certificate> > certs;

	for (int i = 0; i < certCount && !error; i++) {

		shared_ptr <vmime::security::cert::X509Certificate> cert =
			vmime::security::cert::X509Certificate_OpenSSL::importInternal(sk_X509_value(chain, i));

		if (cert) {
			certs.push_back(cert);
		} else {
			error = true;
		}
	}

	if (error) {
		return null;
	}

	return make_shared <security::cert::certificateChain>(certs);
}


void TLSSocket_OpenSSL::internalThrow() {

	if (m_ex.get()) {
		throw *m_ex;
	}
}


void TLSSocket_OpenSSL::handleError(int rc) {

	if (rc > 0) {
		return;
	}

	internalThrow();

	int  sslError   = SSL_get_error(m_ssl, rc);
	long lastError  = ERR_get_error();

	switch (sslError) {

		case SSL_ERROR_ZERO_RETURN:

			disconnect();
			return;

		case SSL_ERROR_SYSCALL: {

			if (lastError == 0) {

				if (rc == 0) {

					throw exceptions::tls_exception("SSL connection unexpectedly closed");

				} else {

					std::ostringstream oss;
					oss << "The BIO reported an error: " << rc;
					oss.flush();
					throw exceptions::tls_exception(oss.str());
				}
			}

			break;
		}

		case SSL_ERROR_WANT_READ:

			BIO_set_retry_read(SSL_get_rbio(m_ssl));
			break;

		case SSL_ERROR_WANT_WRITE:

			BIO_set_retry_write(SSL_get_wbio(m_ssl));
			break;

		// This happens only for BIOs of type BIO_s_connect() or BIO_s_accept()
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
		// SSL_CTX_set_client_cert_cb related, not used
		case SSL_ERROR_WANT_X509_LOOKUP:
		case SSL_ERROR_SSL:
		default:

			if (lastError == 0) {

				throw exceptions::tls_exception("Unexpected SSL IO error");

			} else {

				char buffer[256];
				ERR_error_string_n(lastError, buffer, sizeof(buffer));
				vmime::string msg(buffer);
				throw exceptions::tls_exception(msg);
			}

			break;
	}
}


unsigned int TLSSocket_OpenSSL::getStatus() const {

	return m_status;
}


// Implementation of custom BIO methods


// static
int TLSSocket_OpenSSL::bio_write(BIO* bio, const char* buf, int len) {

	BIO_clear_retry_flags(bio);

	if (buf == NULL || len <= 0) {
		return -1;
	}

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(BIO_get_data(bio));

	if (!BIO_get_init(bio) || !sok) {
		return -1;
	}

	try {

		const size_t n = sok->m_wrapped->sendRawNonBlocking(
			reinterpret_cast <const byte_t*>(buf), len
		);

		if (n == 0 && sok->m_wrapped->getStatus() & socket::STATUS_WOULDBLOCK) {
			BIO_set_retry_write(bio);
			return -1;
		}

		return static_cast <int>(n);

	} catch (exception& e) {

		// Workaround for passing C++ exceptions from C BIO functions
		sok->m_ex.reset(e.clone());
		return -1;
	}
}


// static
int TLSSocket_OpenSSL::bio_read(BIO* bio, char* buf, int len) {

	BIO_clear_retry_flags(bio);

	if (buf == NULL || len <= 0) {
		return -1;
	}

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(BIO_get_data(bio));

	if (!BIO_get_init(bio) || !sok) {
		return -1;
	}

	try {

		const size_t n = sok->m_wrapped->receiveRaw(
			reinterpret_cast <byte_t*>(buf), len
		);

		if (n == 0 || sok->m_wrapped->getStatus() & socket::STATUS_WOULDBLOCK) {
			BIO_set_retry_read(bio);
			return -1;
		}

		return static_cast <int>(n);

	} catch (exception& e) {

		// Workaround for passing C++ exceptions from C BIO functions
		sok->m_ex.reset(e.clone());
		return -1;
	}
}


// static
int TLSSocket_OpenSSL::bio_puts(BIO* bio, const char* str) {

	return bio_write(bio, str, static_cast <int>(strlen(str)));
}


// static
long TLSSocket_OpenSSL::bio_ctrl(BIO* bio, int cmd, long num, void* /* ptr */) {

	long ret = 1;

	switch (cmd) {

		case BIO_CTRL_INFO:

			ret = 0;
			break;

		case BIO_CTRL_GET_CLOSE:

			ret = BIO_get_shutdown(bio);
			break;

		case BIO_CTRL_SET_CLOSE:

			BIO_set_shutdown(bio, static_cast <int>(num));
			break;

		case BIO_CTRL_PENDING:
		case BIO_CTRL_WPENDING:

			ret = 0;
			break;

		case BIO_CTRL_DUP:
		case BIO_CTRL_FLUSH:

			ret = 1;
			break;

		default:

			ret = 0;
			break;
	}

	return ret;
}


// static
int TLSSocket_OpenSSL::bio_create(BIO* bio) {

	BIO_set_init(bio, 0);
	BIO_set_num(bio, 0);
	BIO_set_data(bio, NULL);
	BIO_set_flags(bio, 0);

	return 1;
}


// static
int TLSSocket_OpenSSL::bio_destroy(BIO* bio) {

	if (!bio) {
		return 0;
	}

	if (BIO_get_shutdown(bio)) {
		BIO_set_data(bio, NULL);
		BIO_set_init(bio, 0);
		BIO_set_flags(bio, 0);
	}

	return 1;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL
