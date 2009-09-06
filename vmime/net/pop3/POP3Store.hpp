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

#ifndef VMIME_NET_POP3_POP3STORE_HPP_INCLUDED
#define VMIME_NET_POP3_POP3STORE_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/messageId.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/net/pop3/POP3ServiceInfos.hpp"

#include "vmime/utility/stream.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Folder;


/** POP3 store service.
  */

class POP3Store : public store
{
	friend class POP3Folder;
	friend class POP3Message;

public:

	POP3Store(ref <session> sess, ref <security::authenticator> auth, const bool secured = false);
	~POP3Store();

	const string getProtocolName() const;

	ref <folder> getDefaultFolder();
	ref <folder> getRootFolder();
	ref <folder> getFolder(const folder::path& path);

	bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	int getCapabilities() const;

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

private:

	enum ResponseCode
	{
		RESPONSE_OK = 0,
		RESPONSE_READY,
		RESPONSE_ERR
	};

	void authenticate(const messageId& randomMID);
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	const std::vector <string> getCapabilities();

	static bool isSuccessResponse(const string& buffer);
	static bool stripFirstLine(const string& buffer, string& result, string* firstLine = NULL);
	static void stripResponseCode(const string& buffer, string& result);
	static int getResponseCode(const string& buffer);

	void sendRequest(const string& buffer, const bool end = true);
	void readResponse(string& buffer, const bool multiLine, utility::progressListener* progress = NULL);
	void readResponse(utility::outputStream& os, utility::progressListener* progress = NULL, const int predictedSize = 0);

	static bool checkTerminator(string& buffer, const bool multiLine);
	static bool checkOneTerminator(string& buffer, const string& term);

	void internalDisconnect();


	void registerFolder(POP3Folder* folder);
	void unregisterFolder(POP3Folder* folder);

	std::list <POP3Folder*> m_folders;


	ref <socket> m_socket;
	bool m_authentified;

	ref <timeoutHandler> m_timeoutHandler;

	const bool m_isPOP3S;

	bool m_secured;
	ref <connectionInfos> m_cntInfos;


	// Service infos
	static POP3ServiceInfos sm_infos;
};


} // pop3
} // net
} // vmime


#endif // VMIME_NET_POP3_POP3STORE_HPP_INCLUDED
