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

#include "tests/testUtils.hpp"


#define VMIME_TEST_SUITE         SMTPTransportTest
#define VMIME_TEST_SUITE_MODULE  "Net/SMTP"


class greetingErrorSMTPTestSocket;
class MAILandRCPTSMTPTestSocket;


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGreetingError)
		VMIME_TEST(testMAILandRCPT)
	VMIME_TEST_LIST_END


	void testGreetingError()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <greetingErrorSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		VASSERT_THROW("Connection", tr->connect(),
			vmime::exceptions::connection_greeting_error);
	}

	void testMAILandRCPT()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <MAILandRCPTSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		VASSERT_NO_THROW("Connection", tr->connect());

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient1@test.vmime.org"));
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient2@test.vmime.org"));
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient3@test.vmime.org"));

		vmime::string data("Message data");
		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(exp, recips, is, 0);
	}

VMIME_TEST_SUITE_END


/** Accepts connection and fails on greeting.
  */
class greetingErrorSMTPTestSocket : public lineBasedTestSocket
{
public:

	void onConnected()
	{
		localSend("421 test.vmime.org Service not available, closing transmission channel\r\n");
		disconnect();
	}

	void processCommand()
	{
		if (!haveMoreLines())
			return;

		getNextLine();

		localSend("502 Command not implemented\r\n");
		processCommand();
	}
};


/** SMTP test server 1.
  *
  * Test send().
  * Ensure MAIL and RCPT commands are sent correctly.
  */
class MAILandRCPTSMTPTestSocket : public lineBasedTestSocket
{
public:

	MAILandRCPTSMTPTestSocket()
	{
		m_recipients.insert("recipient1@test.vmime.org");
		m_recipients.insert("recipient2@test.vmime.org");
		m_recipients.insert("recipient3@test.vmime.org");

		m_state = STATE_NOT_CONNECTED;
	}

	void onConnected()
	{
		localSend("220 test.vmime.org Service ready\r\n");
		processCommand();

		m_state = STATE_COMMAND;
	}

	void processCommand()
	{
		if (!haveMoreLines())
			return;

		vmime::string line = getNextLine();
		std::istringstream iss(line);

		switch (m_state)
		{
		case STATE_NOT_CONNECTED:

			localSend("451 Requested action aborted: invalid state\r\n");
			break;

		case STATE_COMMAND:
		{
			std::string cmd;
			iss >> cmd;

			if (cmd.empty())
			{
				localSend("500 Syntax error, command unrecognized\r\n");
			}
			else if (cmd == "HELO")
			{
				localSend("250 OK\r\n");
			}
			else if (cmd == "MAIL")
			{
				VASSERT_EQ("MAIL", std::string("MAIL FROM:<expeditor@test.vmime.org>"), line);

				localSend("250 OK\r\n");
			}
			else if (cmd == "RCPT")
			{
				const vmime::string::size_type lt = line.find('<');
				const vmime::string::size_type gt = line.find('>');

				VASSERT("RCPT <", lt != vmime::string::npos);
				VASSERT("RCPT >", gt != vmime::string::npos);
				VASSERT("RCPT ><", gt >= lt);

				const vmime::string recip = vmime::string
					(line.begin() + lt + 1, line.begin() + gt);

				std::set <vmime::string>::iterator it =
					m_recipients.find(recip);

				VASSERT(std::string("Recipient not found: '") + recip + "'",
					it != m_recipients.end());

				m_recipients.erase(it);

				localSend("250 OK, recipient accepted\r\n");
			}
			else if (cmd == "DATA")
			{
				VASSERT("All recipients", m_recipients.empty());

				localSend("354 Ready to accept data; end with <CRLF>.<CRLF>\r\n");

				m_state = STATE_DATA;
				m_msgData.clear();
			}
			else if (cmd == "NOOP")
			{
				localSend("250 Completed\r\n");
			}
			else if (cmd == "QUIT")
			{
				localSend("221 test.vmime.org Service closing transmission channel\r\n");
			}
			else
			{
				localSend("502 Command not implemented\r\n");
			}

			break;
		}
		case STATE_DATA:
		{
			if (line == ".")
			{
				VASSERT_EQ("Data", "Message data\r\n", m_msgData);

				localSend("250 Message accepted for delivery\r\n");
				m_state = STATE_COMMAND;
			}
			else
			{
				m_msgData += line + "\r\n";
			}

			break;
		}

		}

		processCommand();
	}

private:

	enum State
	{
		STATE_NOT_CONNECTED,
		STATE_COMMAND,
		STATE_DATA
	};

	int m_state;

	std::set <vmime::string> m_recipients;

	std::string m_msgData;
};


