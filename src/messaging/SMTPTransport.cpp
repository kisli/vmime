//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/messaging/SMTPTransport.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"
#include "vmime/encoderB64.hpp"
#include "vmime/message.hpp"
#include "vmime/mailboxList.hpp"

#include "vmime/messaging/authHelper.hpp"


namespace vmime {
namespace messaging {


SMTPTransport::SMTPTransport(session* sess, authenticator* auth)
	: transport(sess, getInfosInstance(), auth), m_socket(NULL),
	  m_authentified(false), m_extendedSMTP(false), m_timeoutHandler(NULL)
{
}


SMTPTransport::~SMTPTransport()
{
	if (isConnected())
		disconnect();
	else if (m_socket)
		internalDisconnect();
}


const string SMTPTransport::getProtocolName() const
{
	return "smtp";
}


void SMTPTransport::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	const string address = getSession()->getProperties()[sm_infos.getPropertyPrefix() + "server.address"];
	const port_t port = getSession()->getProperties().getProperty(sm_infos.getPropertyPrefix() + "server.port", sm_infos.getDefaultPort());

	// Create the time-out handler
	if (getSession()->getProperties().hasProperty
		(sm_infos.getPropertyPrefix() + "timeout.factory"))
	{
		timeoutHandlerFactory* tof = platformDependant::getHandler()->
			getTimeoutHandlerFactory(getSession()->getProperties()
				[sm_infos.getPropertyPrefix() + "timeout.factory"]);

		m_timeoutHandler = tof->create();
	}

	// Create and connect the socket
	socketFactory* sf = platformDependant::getHandler()->getSocketFactory
		(getSession()->getProperties().getProperty(sm_infos.getPropertyPrefix() + "server.socket-factory", string("default")));

	m_socket = sf->create();
	m_socket->connect(address, port);

	// Connection
	//
	// eg:  C: <connection to server>
	// ---  S: 220 smtp.domain.com Service ready

	string response;
	readResponse(response);

	if (responseCode(response) != 220)
	{
		internalDisconnect();
		throw exceptions::connection_greeting_error(response);
	}

	// Identification
	// First, try Extended SMTP (ESMTP)
	//
	// eg:  C: EHLO thismachine.ourdomain.com
	//      S: 250 OK

	sendRequest("EHLO " + platformDependant::getHandler()->getHostName());
	readResponse(response);

	if (responseCode(response) != 250)
	{
		// Next, try "Basic" SMTP
		//
		// eg:  C: HELO thismachine.ourdomain.com
		//      S: 250 OK

		sendRequest("HELO " + platformDependant::getHandler()->getHostName());
		readResponse(response);

		if (responseCode(response) != 250)
		{
			internalDisconnect();
			throw exceptions::connection_greeting_error(response);
		}

		m_extendedSMTP = false;
	}
	else
	{
		m_extendedSMTP = true;
	}

	// Authentication
	if (getSession()->getProperties().getProperty
		(sm_infos.getPropertyPrefix() + "options.need-authentication", false) == true)
	{
		if (!m_extendedSMTP)
		{
			internalDisconnect();
			throw exceptions::command_error("AUTH", "ESMTP not supported.");
		}

		const authenticationInfos auth = getAuthenticator()->requestAuthInfos();
		bool authentified = false;

		enum AuthMethods
		{
			First = 0,
			CRAM_MD5 = First,
			// TODO: more authentication methods...
			End
		};

		for (int currentMethod = First ; !authentified ; ++currentMethod)
		{
			switch (currentMethod)
			{
			case CRAM_MD5:
			{
				sendRequest("AUTH CRAM-MD5");
				readResponse(response);

				if (responseCode(response) == 334)
				{
					encoderB64 base64;

					string challengeB64 = responseText(response);
					string challenge, challengeHex;

					{
						utility::inputStreamStringAdapter in(challengeB64);
						utility::outputStreamStringAdapter out(challenge);

						base64.decode(in, out);
					}

					hmac_md5(challenge, auth.getPassword(), challengeHex);

					string decoded = auth.getUsername() + " " + challengeHex;
					string encoded;

					{
						utility::inputStreamStringAdapter in(decoded);
						utility::outputStreamStringAdapter out(encoded);

						base64.encode(in, out);
					}

					sendRequest(encoded);
					readResponse(response);

					if (responseCode(response) == 235)
					{
						authentified = true;
					}
					else
					{
						internalDisconnect();
						throw exceptions::authentication_error(response);
					}
				}

				break;
			}
			case End:
			{
				// All authentication methods have been tried and
				// the server does not understand any.
				throw exceptions::authentication_error(response);
			}

			}
		}
	}

	m_authentified = true;
}


const bool SMTPTransport::isConnected() const
{
	return (m_socket && m_socket->isConnected() && m_authentified);
}


void SMTPTransport::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void SMTPTransport::internalDisconnect()
{
	sendRequest("QUIT");

	m_socket->disconnect();

	delete (m_socket);
	m_socket = NULL;

	delete (m_timeoutHandler);
	m_timeoutHandler = NULL;

	m_authentified = false;
	m_extendedSMTP = false;
}


void SMTPTransport::noop()
{
	m_socket->send("NOOP");

	string response;
	readResponse(response);

	if (responseCode(response) != 250)
		throw exceptions::command_error("NOOP", response);
}


static void extractMailboxes
	(mailboxList& recipients, const addressList& list)
{
	for (int i = 0 ; i < list.getAddressCount() ; ++i)
	{
		mailbox* mbox = dynamic_cast <mailbox*>(list.getAddressAt(i)->clone());

		if (mbox != NULL)
			recipients.appendMailbox(mbox);
	}
}


void SMTPTransport::send(vmime::message* msg, progressionListener* progress)
{
	// Extract expeditor
	mailbox expeditor;

	try
	{
		const mailboxField& from = dynamic_cast <const mailboxField&>
			(*msg->getHeader()->findField(fields::FROM));
		expeditor = from.getValue();
	}
	catch (exceptions::no_such_field&)
	{
		throw exceptions::no_expeditor();
	}

	// Extract recipients
	mailboxList recipients;

	try
	{
		const addressListField& to = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::TO));
		extractMailboxes(recipients, to.getValue());
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressListField& cc = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::CC));
		extractMailboxes(recipients, cc.getValue());
	}
	catch (exceptions::no_such_field&) { }

	try
	{
		const addressListField& bcc = dynamic_cast <const addressListField&>
			(*msg->getHeader()->findField(fields::BCC));
		extractMailboxes(recipients, bcc.getValue());
	}
	catch (exceptions::no_such_field&) { }

	// Generate the message, "stream" it and delegate the sending
	// to the generic send() function.
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str(oss.str());

	utility::inputStreamStringAdapter isAdapter(str);

	send(expeditor, recipients, isAdapter, str.length(), progress);
}


void SMTPTransport::send(const mailbox& expeditor, const mailboxList& recipients,
                         utility::inputStream& is, const utility::stream::size_type size,
                         progressionListener* progress)
{
	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty())
		throw exceptions::no_recipient();
	else if (expeditor.isEmpty())
		throw exceptions::no_expeditor();

	// Emit the "MAIL" command
	string response;

	sendRequest("MAIL FROM: <" + expeditor.getEmail() + ">");
	readResponse(response);

	if (responseCode(response) != 250)
	{
		internalDisconnect();
		throw exceptions::command_error("MAIL", response);
	}

	// Emit a "RCPT TO" command for each recipient
	for (int i = 0 ; i < recipients.getMailboxCount() ; ++i)
	{
		const mailbox& mbox = *recipients.getMailboxAt(i);

		sendRequest("RCPT TO: <" + mbox.getEmail() + ">");
		readResponse(response);

		if (responseCode(response) != 250)
		{
			internalDisconnect();
			throw exceptions::command_error("RCPT TO", response);
		}
	}

	// Send the message data
	sendRequest("DATA");
	readResponse(response);

	if (responseCode(response) != 354)
	{
		internalDisconnect();
		throw exceptions::command_error("DATA", response);
	}

	int current = 0, total = size;

	if (progress)
		progress->start(total);

	char buffer[65536];

	while (!is.eof())
	{
		const int read = is.read(buffer, sizeof(buffer));

		// Transform '.' into '..' at the beginning of a line
		char* start = buffer;
		char* end = buffer + read;
		char* pos = buffer;

		while ((pos = std::find(pos, end, '.')) != end)
		{
			if (pos > buffer && *(pos - 1) == '\n')
			{
				m_socket->sendRaw(start, pos - start);
				m_socket->sendRaw(".", 1);

				start = pos;
			}

			++pos;
		}

		// Send the remaining data
		m_socket->sendRaw(start, end - start);

		current += read;

		// Notify progression
		if (progress)
		{
			total = std::max(total, current);
			progress->progress(current, total);
		}
	}

	if (progress)
		progress->stop(total);

	m_socket->sendRaw("\r\n.\r\n", 5);
	readResponse(response);

	if (responseCode(response) != 250)
	{
		internalDisconnect();
		throw exceptions::command_error("DATA", response);
	}
}


void SMTPTransport::sendRequest(const string& buffer, const bool end)
{
	m_socket->send(buffer);
	if (end) m_socket->send("\r\n");
}


const int SMTPTransport::responseCode(const string& response)
{
	int code = 0;

	if (response.length() >= 3)
	{
		code = (response[0] - '0') * 100
		     + (response[1] - '0') * 10
		     + (response[2] - '0');
	}

	return (code);
}


const string SMTPTransport::responseText(const string& response)
{
	string text;

	std::istringstream iss(response);
	std::string line;

	while (std::getline(iss, line))
	{
		if (line.length() >= 4)
			text += line.substr(4);
		else
			text += line;

		text += "\n";
	}

	return (text);
}


void SMTPTransport::readResponse(string& buffer)
{
	bool foundTerminator = false;

	buffer.clear();

	for ( ; !foundTerminator ; )
	{
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

		// Append the data to the response buffer
		buffer += receiveBuffer;

		// Check for terminator string (and strip it if present)
		if (buffer.length() >= 2 && buffer[buffer.length() - 1] == '\n')
		{
			string::size_type p = buffer.length() - 2;
			bool end = false;

			for ( ; !end ; --p)
			{
				if (p == 0 || buffer[p] == '\n')
				{
					end = true;

					if (p + 4 < buffer.length())
						foundTerminator = true;
				}
			}
		}
	}

	// Remove [CR]LF at the end of the response
	if (buffer.length() >= 2 && buffer[buffer.length() - 1] == '\n')
	{
		if (buffer[buffer.length() - 2] == '\r')
			buffer.resize(buffer.length() - 2);
		else
			buffer.resize(buffer.length() - 1);
	}
}



// Service infos

SMTPTransport::_infos SMTPTransport::sm_infos;


const serviceInfos& SMTPTransport::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& SMTPTransport::getInfos() const
{
	return (sm_infos);
}


const port_t SMTPTransport::_infos::getDefaultPort() const
{
	return (25);
}


const string SMTPTransport::_infos::getPropertyPrefix() const
{
	return "transport.smtp.";
}


const std::vector <string> SMTPTransport::_infos::getAvailableProperties() const
{
	std::vector <string> list;

	// SMTP-specific options
	list.push_back("options.need-authentication");

	// Common properties
	list.push_back("auth.username");
	list.push_back("auth.password");

	list.push_back("server.address");
	list.push_back("server.port");
	list.push_back("server.socket-factory");

	list.push_back("timeout.factory");

	return (list);
}


} // messaging
} // vmime
