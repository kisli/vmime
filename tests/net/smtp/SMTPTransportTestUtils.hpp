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
		m_ehloSent = m_heloSent = m_mailSent = m_rcptSent = m_dataSent = m_quitSent = false;
	}

	~MAILandRCPTSMTPTestSocket()
	{
		VASSERT("Client must send the DATA command", m_dataSent);
		VASSERT("Client must send the QUIT command", m_quitSent);
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
			else if (cmd == "EHLO")
			{
				localSend("502 Command not implemented\r\n");

				m_ehloSent = true;
			}
			else if (cmd == "HELO")
			{
				VASSERT("Client must send the EHLO command before HELO", m_ehloSent);

				localSend("250 OK\r\n");

				m_heloSent = true;
			}
			else if (cmd == "MAIL")
			{
				VASSERT("Client must send the HELO command", m_heloSent);
				VASSERT("The MAIL command must be sent only one time", !m_mailSent);

				VASSERT_EQ("MAIL", std::string("MAIL FROM:<expeditor@test.vmime.org>"), line);

				localSend("250 OK\r\n");

				m_mailSent = true;
			}
			else if (cmd == "RCPT")
			{
				const vmime::size_t lt = line.find('<');
				const vmime::size_t gt = line.find('>');

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

				m_rcptSent = true;
			}
			else if (cmd == "DATA")
			{
				VASSERT("Client must send the MAIL command", m_mailSent);
				VASSERT("Client must send the RCPT command", m_rcptSent);
				VASSERT("All recipients", m_recipients.empty());

				localSend("354 Ready to accept data; end with <CRLF>.<CRLF>\r\n");

				m_state = STATE_DATA;
				m_msgData.clear();

				m_dataSent = true;
			}
			else if (cmd == "NOOP")
			{
				localSend("250 Completed\r\n");
			}
			else if (cmd == "QUIT")
			{
				m_quitSent = true;

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

	bool m_ehloSent, m_heloSent, m_mailSent, m_rcptSent,
	     m_dataSent, m_quitSent;
};



/** SMTP test server 2.
  *
  * Test CHUNKING extension/BDAT command.
  */
class chunkingSMTPTestSocket : public testSocket
{
public:

	chunkingSMTPTestSocket()
	{
		m_state = STATE_NOT_CONNECTED;
		m_bdatChunkCount = 0;
		m_ehloSent = m_mailSent = m_rcptSent = m_quitSent = false;
	}

	~chunkingSMTPTestSocket()
	{
		VASSERT_EQ("BDAT chunk count", 3, m_bdatChunkCount);
		VASSERT("Client must send the QUIT command", m_quitSent);
	}

	void onConnected()
	{
		localSend("220 test.vmime.org Service ready\r\n");
		processCommand();

		m_state = STATE_COMMAND;
	}

	void onDataReceived()
	{
		if (m_state == STATE_DATA)
		{
			if (m_bdatChunkReceived != m_bdatChunkSize)
			{
				const size_t remaining = m_bdatChunkSize - m_bdatChunkReceived;
				const size_t received = localReceiveRaw(NULL, remaining);

				m_bdatChunkReceived += received;
			}

			if (m_bdatChunkReceived == m_bdatChunkSize)
			{
				m_state = STATE_COMMAND;
			}
		}

		processCommand();
	}

	void processCommand()
	{
		vmime::string line;

		if (!localReceiveLine(line))
			return;

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

			if (cmd == "EHLO")
			{
				localSend("250-test.vmime.org says hello\r\n");
				localSend("250 CHUNKING\r\n");

				m_ehloSent = true;
			}
			else if (cmd == "HELO")
			{
				VASSERT("Client must not send the HELO command, as EHLO succeeded", false);
			}
			else if (cmd == "MAIL")
			{
				VASSERT("The MAIL command must be sent only one time", !m_mailSent);

				localSend("250 OK\r\n");

				m_mailSent = true;
			}
			else if (cmd == "RCPT")
			{
				localSend("250 OK, recipient accepted\r\n");

				m_rcptSent = true;
			}
			else if (cmd == "DATA")
			{
				VASSERT("BDAT must be used here!", false);
			}
			else if (cmd == "BDAT")
			{
				VASSERT("Client must send the MAIL command", m_mailSent);
				VASSERT("Client must send the RCPT command", m_rcptSent);

				unsigned long chunkSize = 0;
				iss >> chunkSize;

				std::string last;
				iss >> last;

				if (m_bdatChunkCount == 0)
				{
					VASSERT_EQ("BDAT chunk1 size", 262144, chunkSize);
					VASSERT_EQ("BDAT chunk1 last", "", last);
				}
				else if (m_bdatChunkCount == 1)
				{
					VASSERT_EQ("BDAT chunk2 size", 262144, chunkSize);
					VASSERT_EQ("BDAT chunk2 last", "", last);
				}
				else if (m_bdatChunkCount == 2)
				{
					VASSERT_EQ("BDAT chunk3 size", 4712, chunkSize);
					VASSERT_EQ("BDAT chunk3 last", "LAST", last);
				}
				else
				{
					VASSERT("No more BDAT command should be issued!", false);
				}

				m_bdatChunkSize = chunkSize;
				m_bdatChunkReceived = 0;
				m_bdatChunkCount++;
				m_state = STATE_DATA;

				localSend("250 chunk received\r\n");
			}
			else if (cmd == "NOOP")
			{
				localSend("250 Completed\r\n");
			}
			else if (cmd == "QUIT")
			{
				localSend("221 test.vmime.org Service closing transmission channel\r\n");

				m_quitSent = true;
			}
			else
			{
				localSend("502 Command not implemented\r\n");
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
	int m_bdatChunkCount;
	int m_bdatChunkSize, m_bdatChunkReceived;

	bool m_ehloSent, m_mailSent, m_rcptSent, m_quitSent;
};


class SMTPTestMessage : public vmime::message
{
public:

	vmime::size_t getChunkBufferSize() const
	{
		static vmime::net::smtp::SMTPChunkingOutputStreamAdapter chunkStream(vmime::null, 0, NULL);
		return chunkStream.getBlockSize();
	}

	const std::vector <vmime::string>& getChunks() const
	{
		static std::vector <vmime::string> chunks;

		if (chunks.size() == 0)
		{
			chunks.push_back(vmime::string(1000, 'A'));
			chunks.push_back(vmime::string(3000, 'B'));
			chunks.push_back(vmime::string(500000, 'C'));
			chunks.push_back(vmime::string(25000, 'D'));
		}

		return chunks;
	}

	void generateImpl
		(const vmime::generationContext& /* ctx */, vmime::utility::outputStream& outputStream,
		 const size_t /* curLinePos */ = 0, size_t* /* newLinePos */ = NULL) const
	{
		for (unsigned int i = 0, n = getChunks().size() ; i < n ; ++i)
		{
			const vmime::string& chunk = getChunks()[i];
			outputStream.write(chunk.data(), chunk.size());
		}
	}
};



/** SMTP test server 3.
  *
  * Test SIZE extension.
  */
template <bool WITH_CHUNKING>
class bigMessageSMTPTestSocket : public testSocket
{
public:

	bigMessageSMTPTestSocket()
	{
		m_state = STATE_NOT_CONNECTED;
		m_ehloSent = m_mailSent = m_rcptSent = m_quitSent = false;
	}

	~bigMessageSMTPTestSocket()
	{
		VASSERT("Client must send the QUIT command", m_quitSent);
	}

	void onConnected()
	{
		localSend("220 test.vmime.org Service ready\r\n");
		processCommand();

		m_state = STATE_COMMAND;
	}

	void onDataReceived()
	{
		processCommand();
	}

	void processCommand()
	{
		vmime::string line;

		if (!localReceiveLine(line))
			return;

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

			if (cmd == "EHLO")
			{
				localSend("250-test.vmime.org says hello\r\n");

				if (WITH_CHUNKING)
					localSend("250-CHUNKING\r\n");

				localSend("250 SIZE 1000000\r\n");

				m_ehloSent = true;
			}
			else if (cmd == "HELO")
			{
				VASSERT("Client must not send the HELO command, as EHLO succeeded", false);
			}
			else if (cmd == "MAIL")
			{
				VASSERT("The MAIL command must be sent only one time", !m_mailSent);

				std::string address;
				iss >> address;

				VASSERT_EQ("MAIL/address", "FROM:<expeditor@test.vmime.org>", address);

				std::string option;
				iss >> option;

				VASSERT_EQ("MAIL/size", "SIZE=4194304", option);

				localSend("552 Channel size limit exceeded\r\n");

				m_mailSent = true;
			}
			else if (cmd == "NOOP")
			{
				localSend("250 Completed\r\n");
			}
			else if (cmd == "QUIT")
			{
				localSend("221 test.vmime.org Service closing transmission channel\r\n");

				m_quitSent = true;
			}
			else
			{
				VASSERT("No other command should be sent", false);

				localSend("502 Command not implemented\r\n");
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

	bool m_ehloSent, m_mailSent, m_rcptSent, m_quitSent;
};


template <unsigned long SIZE>
class SMTPBigTestMessage : public vmime::message
{
public:

	size_t getGeneratedSize(const vmime::generationContext& /* ctx */)
	{
		return SIZE;
	}

	void generateImpl(const vmime::generationContext& /* ctx */,
		 vmime::utility::outputStream& outputStream,
		 const vmime::size_t /* curLinePos */ = 0,
		 vmime::size_t* /* newLinePos */ = NULL) const
	{
		for (unsigned int i = 0, n = SIZE ; i < n ; ++i)
			outputStream.write("X", 1);
	}
};

typedef SMTPBigTestMessage <4194304> SMTPBigTestMessage4MB;
