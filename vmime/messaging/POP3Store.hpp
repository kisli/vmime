//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_MESSAGING_POP3STORE_HPP_INCLUDED
#define VMIME_MESSAGING_POP3STORE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/messaging/store.hpp"
#include "vmime/messaging/socket.hpp"
#include "vmime/messaging/timeoutHandler.hpp"

#include "vmime/utility/stream.hpp"


namespace vmime {
namespace messaging {


/** POP3 store service.
  */

class POP3Store : public store
{
	friend class POP3Folder;
	friend class POP3Message;

public:

	POP3Store(session* sess, authenticator* auth);
	~POP3Store();

	const string getProtocolName() const;

	folder* getDefaultFolder();
	folder* getRootFolder();
	folder* getFolder(const folder::path& path);

	const bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	const bool isConnected() const;
	void disconnect();

	void noop();

	const int getCapabilities() const;

private:

	static const bool isSuccessResponse(const string& buffer);
	static const bool stripFirstLine(const string& buffer, string& result, string* firstLine = NULL);
	static void stripResponseCode(const string& buffer, string& result);

	void sendRequest(const string& buffer, const bool end = true);
	void readResponse(string& buffer, const bool multiLine, progressionListener* progress = NULL);
	void readResponse(utility::outputStream& os, progressionListener* progress = NULL, const int predictedSize = 0);

	static const bool checkTerminator(string& buffer, const bool multiLine);
	static const bool checkOneTerminator(string& buffer, const string& term);

	void internalDisconnect();


	void registerFolder(POP3Folder* folder);
	void unregisterFolder(POP3Folder* folder);

	std::list <POP3Folder*> m_folders;


	socket* m_socket;
	bool m_authentified;

	timeoutHandler* m_timeoutHandler;


	// Service infos
	class _infos : public serviceInfos
	{
	public:

		const port_t getDefaultPort() const;

		const string getPropertyPrefix() const;
		const std::vector <string> getAvailableProperties() const;
	};

	static _infos sm_infos;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_POP3STORE_HPP_INCLUDED
