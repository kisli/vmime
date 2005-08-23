//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3Folder.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"
#include "vmime/messageId.hpp"
#include "vmime/utility/md5.hpp"
#include "vmime/utility/filteredStream.hpp"

#include <algorithm>


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(sm_infos.getPropertyValue <type>(getSession(), sm_infos.getProperties().prop))
#define HAS_PROPERTY(prop) \
	(sm_infos.hasProperty(getSession(), sm_infos.getProperties().prop))


namespace vmime {
namespace net {
namespace pop3 {


POP3Store::POP3Store(ref <session> sess, ref <authenticator> auth)
	: store(sess, getInfosInstance(), auth), m_socket(NULL),
	  m_authentified(false), m_timeoutHandler(NULL)
{
}


POP3Store::~POP3Store()
{
	if (isConnected())
		disconnect();
	else if (m_socket)
		internalDisconnect();
}


const string POP3Store::getProtocolName() const
{
	return "pop3";
}


ref <folder> POP3Store::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(folder::path(folder::path::component("INBOX")), this);
}


ref <folder> POP3Store::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(folder::path(), this);
}


ref <folder> POP3Store::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <POP3Folder>(path, this);
}


const bool POP3Store::isValidFolderName(const folder::path::component& /* name */) const
{
	return true;
}


void POP3Store::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = GET_PROPERTY(string, PROPERTY_SERVER_ADDRESS);
	const port_t port = GET_PROPERTY(port_t, PROPERTY_SERVER_PORT);

	// Create the time-out handler
	if (HAS_PROPERTY(PROPERTY_TIMEOUT_FACTORY))
	{
		timeoutHandlerFactory* tof = platformDependant::getHandler()->
			getTimeoutHandlerFactory(GET_PROPERTY(string, PROPERTY_TIMEOUT_FACTORY));

		m_timeoutHandler = tof->create();
	}

	// Create and connect the socket
	socketFactory* sf = platformDependant::getHandler()->
		getSocketFactory(GET_PROPERTY(string, PROPERTY_SERVER_SOCKETFACTORY));

	m_socket = sf->create();
	m_socket->connect(address, port);

	// Connection
	//
	// eg:  C: <connection to server>
	// ---  S: +OK MailSite POP3 Server 5.3.4.0 Ready <36938848.1056800841.634@somewhere.com>

	string response;
	readResponse(response, false);

	if (isSuccessResponse(response))
	{
		bool authentified = false;

		const authenticationInfos auth = getAuthenticator()->requestAuthInfos();

		// Secured authentication with APOP (if requested and if available)
		//
		// eg:  C: APOP vincent <digest>
		// ---  S: +OK vincent is a valid mailbox
		messageId mid(response);

		if (GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP))
		{
			if (mid.getLeft().length() && mid.getRight().length())
			{
				// <digest> is the result of MD5 applied to "<message-id>password"
				sendRequest("APOP " + auth.getUsername() + " "
					+ utility::md5(mid.generate() + auth.getPassword()).hex());
				readResponse(response, false);

				if (isSuccessResponse(response))
				{
					authentified = true;
				}
				else
				{
					if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP_FALLBACK))
					{
						internalDisconnect();
						throw exceptions::authentication_error(response);
					}
				}
			}
			else
			{
				// APOP not supported
				if (!GET_PROPERTY(bool, PROPERTY_OPTIONS_APOP_FALLBACK))
				{
					// Can't fallback on basic authentification
					internalDisconnect();
					throw exceptions::unsupported_option();
				}
			}
		}

		if (!authentified)
		{
			// Basic authentication
			//
			// eg:  C: USER vincent
			// ---  S: +OK vincent is a valid mailbox
			//
			//      C: PASS couic
			//      S: +OK vincent's maildrop has 2 messages (320 octets)

			sendRequest("USER " + auth.getUsername());
			readResponse(response, false);

			if (isSuccessResponse(response))
			{
				sendRequest("PASS " + auth.getPassword());
				readResponse(response, false);

				if (!isSuccessResponse(response))
				{
					internalDisconnect();
					throw exceptions::authentication_error(response);
				}
			}
			else
			{
				internalDisconnect();
				throw exceptions::authentication_error(response);
			}
		}
	}
	else
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(response);
	}

	m_authentified = true;
}


const bool POP3Store::isConnected() const
{
	return (m_socket && m_socket->isConnected() && m_authentified);
}


void POP3Store::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void POP3Store::internalDisconnect()
{
	for (std::list <POP3Folder*>::iterator it = m_folders.begin() ;
	     it != m_folders.end() ; ++it)
	{
		(*it)->onStoreDisconnected();
	}

	m_folders.clear();


	sendRequest("QUIT");

	m_socket->disconnect();
	m_socket = NULL;

	m_timeoutHandler = NULL;

	m_authentified = false;
}


void POP3Store::noop()
{
	m_socket->send("NOOP");

	string response;
	readResponse(response, false);

	if (!isSuccessResponse(response))
		throw exceptions::command_error("NOOP", response);
}


const bool POP3Store::isSuccessResponse(const string& buffer)
{
	static const string OK("+OK");

	return (buffer.length() >= 3 &&
	        std::equal(buffer.begin(), buffer.begin() + 3, OK.begin()));
}


const bool POP3Store::stripFirstLine(const string& buffer, string& result, string* firstLine)
{
	const string::size_type end = buffer.find('\n');

	if (end != string::npos)
	{
		if (firstLine) *firstLine = buffer.substr(0, end);
		result = buffer.substr(end + 1);
		return (true);
	}
	else
	{
		result = buffer;
		return (false);
	}
}


void POP3Store::stripResponseCode(const string& buffer, string& result)
{
	const string::size_type pos = buffer.find_first_of(" \t");

	if (pos != string::npos)
		result = buffer.substr(pos + 1);
	else
		result = buffer;
}


void POP3Store::sendRequest(const string& buffer, const bool end)
{
	if (end)
		m_socket->send(buffer + "\r\n");
	else
		m_socket->send(buffer);
}


void POP3Store::readResponse(string& buffer, const bool multiLine,
                             utility::progressionListener* progress)
{
	bool foundTerminator = false;
	int current = 0, total = 0;

	if (progress)
		progress->start(total);

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	buffer.clear();

	string::value_type last1 = '\0', last2 = '\0';

	for ( ; !foundTerminator ; )
	{
#if 0 // not supported
		// Check for possible cancellation
		if (progress && progress->cancel())
			throw exceptions::operation_cancelled();
#endif

		// Check whether the time-out delay is elapsed
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
				throw exceptions::operation_timed_out();
		}

		// Receive data from the socket
		string receiveBuffer;
		m_socket->receive(receiveBuffer);

		if (receiveBuffer.empty())   // buffer is empty
		{
			platformDependant::getHandler()->wait();
			continue;
		}

		// We have received data: reset the time-out counter
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		// Check for transparent characters: '\n..' becomes '\n.'
		const string::value_type first = receiveBuffer[0];

		if (first == '.' && last2 == '\n' && last1 == '.')
		{
			receiveBuffer.erase(receiveBuffer.begin());
		}
		else if (receiveBuffer.length() >= 2 && first == '.' &&
		         receiveBuffer[1] == '.' && last1 == '\n')
		{
			receiveBuffer.erase(receiveBuffer.begin());
		}

		for (string::size_type trans ;
		     string::npos != (trans = receiveBuffer.find("\n..")) ; )
		{
			receiveBuffer.replace(trans, 3, "\n.");
		}

		last1 = receiveBuffer[receiveBuffer.length() - 1];
		last2 = (receiveBuffer.length() >= 2) ? receiveBuffer[receiveBuffer.length() - 2] : 0;

		// Append the data to the response buffer
		buffer += receiveBuffer;
		current += receiveBuffer.length();

		// Check for terminator string (and strip it if present)
		foundTerminator = checkTerminator(buffer, multiLine);

		// Notify progression
		if (progress)
		{
			total = std::max(total, current);
			progress->progress(current, total);
		}

		// If there is an error (-ERR) when executing a command that
		// requires a multi-line response, the error response will
		// include only one line, so we stop waiting for a multi-line
		// terminator and check for a "normal" one.
		if (multiLine && !foundTerminator && buffer.length() >= 4 && buffer[0] == '-')
		{
			foundTerminator = checkTerminator(buffer, false);
		}
	}

	if (progress)
		progress->stop(total);
}


void POP3Store::readResponse(utility::outputStream& os,
	utility::progressionListener* progress, const int predictedSize)
{
	int current = 0, total = predictedSize;

	string temp;
	bool codeDone = false;

	if (progress)
		progress->start(total);

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	utility::inputStreamSocketAdapter sis(*m_socket);
	utility::stopSequenceFilteredInputStream <5> sfis1(sis, "\r\n.\r\n");
	utility::stopSequenceFilteredInputStream <3> sfis2(sfis1, "\n.\n");
	utility::dotFilteredInputStream dfis(sfis2);   // "\n.." --> "\n."

	utility::inputStream& is = dfis;

	while (!is.eof())
	{
#if 0 // not supported
		// Check for possible cancellation
		if (progress && progress->cancel())
			throw exceptions::operation_cancelled();
#endif

		// Check whether the time-out delay is elapsed
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
				throw exceptions::operation_timed_out();
		}

		// Receive data from the socket
		utility::stream::value_type buffer[65536];
		const utility::stream::size_type read = is.read(buffer, sizeof(buffer));

		if (read == 0)   // buffer is empty
		{
			platformDependant::getHandler()->wait();
			continue;
		}

		// We have received data: reset the time-out counter
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		// If we don't have extracted the response code yet
		if (!codeDone)
		{
			temp += string(buffer, read);

			string firstLine;

			if (stripFirstLine(temp, temp, &firstLine) == true)
			{
				if (!isSuccessResponse(firstLine))
					throw exceptions::command_error("?", firstLine);

				codeDone = true;

				os.write(temp.data(), temp.length());
				temp.clear();

				continue;
			}
		}
		else
		{
			// Inject the data into the output stream
			os.write(buffer, read);
			current += read;

			// Notify progression
			if (progress)
			{
				total = std::max(total, current);
				progress->progress(current, total);
			}
		}
	}

	if (progress)
		progress->stop(total);
}


const bool POP3Store::checkTerminator(string& buffer, const bool multiLine)
{
	// Multi-line response
	if (multiLine)
	{
		static const string term1("\r\n.\r\n");
		static const string term2("\n.\n");

		return (checkOneTerminator(buffer, term1) ||
		        checkOneTerminator(buffer, term2));
	}
	// Normal response
	else
	{
		static const string term1("\r\n");
		static const string term2("\n");

		return (checkOneTerminator(buffer, term1) ||
		        checkOneTerminator(buffer, term2));
	}

	return (false);
}


const bool POP3Store::checkOneTerminator(string& buffer, const string& term)
{
	if (buffer.length() >= term.length() &&
		std::equal(buffer.end() - term.length(), buffer.end(), term.begin()))
	{
		buffer.erase(buffer.end() - term.length(), buffer.end());
		return (true);
	}

	return (false);
}


void POP3Store::registerFolder(POP3Folder* folder)
{
	m_folders.push_back(folder);
}


void POP3Store::unregisterFolder(POP3Folder* folder)
{
	std::list <POP3Folder*>::iterator it = std::find(m_folders.begin(), m_folders.end(), folder);
	if (it != m_folders.end()) m_folders.erase(it);
}


const int POP3Store::getCapabilities() const
{
	return (CAPABILITY_DELETE_MESSAGE);
}



// Service infos

POP3Store::_infos POP3Store::sm_infos;


const serviceInfos& POP3Store::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& POP3Store::getInfos() const
{
	return (sm_infos);
}


const string POP3Store::_infos::getPropertyPrefix() const
{
	return "store.pop3.";
}


const POP3Store::_infos::props& POP3Store::_infos::getProperties() const
{
	static props p =
	{
		// POP3-specific options
		property("options.apop", serviceInfos::property::TYPE_BOOL, "false"),
		property("options.apop.fallback", serviceInfos::property::TYPE_BOOL, "false"),

		// Common properties
		property(serviceInfos::property::AUTH_USERNAME, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::AUTH_PASSWORD, serviceInfos::property::FLAG_REQUIRED),

		property(serviceInfos::property::SERVER_ADDRESS, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::SERVER_PORT, "110"),
		property(serviceInfos::property::SERVER_SOCKETFACTORY),

		property(serviceInfos::property::TIMEOUT_FACTORY)
	};

	return p;
}


const std::vector <serviceInfos::property> POP3Store::_infos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	// POP3-specific options
	list.push_back(p.PROPERTY_OPTIONS_APOP);
	list.push_back(p.PROPERTY_OPTIONS_APOP_FALLBACK);

	// Common properties
	list.push_back(p.PROPERTY_AUTH_USERNAME);
	list.push_back(p.PROPERTY_AUTH_PASSWORD);

	list.push_back(p.PROPERTY_SERVER_ADDRESS);
	list.push_back(p.PROPERTY_SERVER_PORT);
	list.push_back(p.PROPERTY_SERVER_SOCKETFACTORY);

	list.push_back(p.PROPERTY_TIMEOUT_FACTORY);

	return (list);
}


} // pop3
} // net
} // vmime

