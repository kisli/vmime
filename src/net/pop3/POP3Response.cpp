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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3Response.hpp"
#include "vmime/net/pop3/POP3Connection.hpp"
#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/platform.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/inputStreamSocketAdapter.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3Response::POP3Response(ref <socket> sok, ref <timeoutHandler> toh)
	: m_socket(sok), m_timeoutHandler(toh)
{
}


// static
ref <POP3Response> POP3Response::readResponse(ref <POP3Connection> conn)
{
	ref <POP3Response> resp = vmime::create <POP3Response>
		(conn->getSocket(), conn->getTimeoutHandler());

	string buffer;
	resp->readResponseImpl(buffer, /* multiLine */ false);

	resp->m_firstLine = buffer;
	resp->m_code = getResponseCode(buffer);
	stripResponseCode(buffer, resp->m_text);

	return resp;
}


// static
ref <POP3Response> POP3Response::readMultilineResponse(ref <POP3Connection> conn)
{
	ref <POP3Response> resp = vmime::create <POP3Response>
		(conn->getSocket(), conn->getTimeoutHandler());

	string buffer;
	resp->readResponseImpl(buffer, /* multiLine */ true);

	string firstLine, nextLines;
	stripFirstLine(buffer, nextLines, &firstLine);

	resp->m_firstLine = firstLine;
	resp->m_code = getResponseCode(firstLine);
	stripResponseCode(firstLine, resp->m_text);

	std::istringstream iss(nextLines);
	string line;

	while (std::getline(iss, line, '\n'))
		resp->m_lines.push_back(utility::stringUtils::trim(line));

	return resp;
}


// static
ref <POP3Response> POP3Response::readLargeResponse
	(ref <POP3Connection> conn, utility::outputStream& os,
	 utility::progressListener* progress, const long predictedSize)
{
	ref <POP3Response> resp = vmime::create <POP3Response>
		(conn->getSocket(), conn->getTimeoutHandler());

	string firstLine;
	resp->readResponseImpl(firstLine, os, progress, predictedSize);

	resp->m_firstLine = firstLine;
	resp->m_code = getResponseCode(firstLine);
	stripResponseCode(firstLine, resp->m_text);

	return resp;
}


bool POP3Response::isSuccess() const
{
	return m_code == CODE_OK;
}


const string POP3Response::getFirstLine() const
{
	return m_firstLine;
}


POP3Response::ResponseCode POP3Response::getCode() const
{
	return m_code;
}


const string POP3Response::getText() const
{
	return m_text;
}


const string POP3Response::getLineAt(const size_t pos) const
{
	return m_lines[pos];
}


size_t POP3Response::getLineCount() const
{
	return m_lines.size();
}


void POP3Response::readResponseImpl(string& buffer, const bool multiLine)
{
	bool foundTerminator = false;

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	buffer.clear();

	string::value_type last1 = '\0', last2 = '\0';

	for ( ; !foundTerminator ; )
	{
		// Check whether the time-out delay is elapsed
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
				throw exceptions::operation_timed_out();

			m_timeoutHandler->resetTimeOut();
		}

		// Receive data from the socket
		string receiveBuffer;
		m_socket->receive(receiveBuffer);

		if (receiveBuffer.empty())   // buffer is empty
		{
			platform::getHandler()->wait();
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
		last2 = static_cast <char>((receiveBuffer.length() >= 2) ? receiveBuffer[receiveBuffer.length() - 2] : 0);

		// Append the data to the response buffer
		buffer += receiveBuffer;

		// Check for terminator string (and strip it if present)
		foundTerminator = checkTerminator(buffer, multiLine);

		// If there is an error (-ERR) when executing a command that
		// requires a multi-line response, the error response will
		// include only one line, so we stop waiting for a multi-line
		// terminator and check for a "normal" one.
		if (multiLine && !foundTerminator && buffer.length() >= 4 && buffer[0] == '-')
		{
			foundTerminator = checkTerminator(buffer, false);
		}
	}
}


void POP3Response::readResponseImpl
	(string& firstLine, utility::outputStream& os,
	 utility::progressListener* progress, const long predictedSize)
{
	long current = 0, total = predictedSize;

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
			platform::getHandler()->wait();
			continue;
		}

		// We have received data: reset the time-out counter
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		// Notify progress
		current += read;

		if (progress)
		{
			total = std::max(total, current);
			progress->progress(current, total);
		}

		// If we don't have extracted the response code yet
		if (!codeDone)
		{
			temp.append(buffer, read);

			string responseData;

			if (stripFirstLine(temp, responseData, &firstLine) == true)
			{
				if (getResponseCode(firstLine) != CODE_OK)
					throw exceptions::command_error("?", firstLine);

				codeDone = true;

				os.write(responseData.data(), responseData.length());
				temp.clear();

				continue;
			}
		}
		else
		{
			// Inject the data into the output stream
			os.write(buffer, read);
		}
	}

	if (progress)
		progress->stop(total);
}


// static
bool POP3Response::stripFirstLine
	(const string& buffer, string& result, string* firstLine)
{
	const string::size_type end = buffer.find('\n');

	if (end != string::npos)
	{
		if (firstLine) *firstLine = utility::stringUtils::trim(buffer.substr(0, end));
		result = buffer.substr(end + 1);
		return true;
	}
	else
	{
		if (firstLine) *firstLine = utility::stringUtils::trim(buffer);
		result = "";
		return false;
	}
}


// static
POP3Response::ResponseCode POP3Response::getResponseCode(const string& buffer)
{
	if (buffer.length() >= 2)
	{
		// +[space]
		if (buffer[0] == '+' &&
		    (buffer[1] == ' ' || buffer[1] == '\t'))
		{
			return CODE_READY;
		}

		// +OK
		if (buffer.length() >= 3)
		{
			if (buffer[0] == '+' &&
			    (buffer[1] == 'O' || buffer[1] == 'o') &&
			    (buffer[2] == 'K' || buffer[1] == 'k'))
			{
				return CODE_OK;
			}
		}
	}

	// -ERR or whatever
	return CODE_ERR;
}


// static
void POP3Response::stripResponseCode(const string& buffer, string& result)
{
	const string::size_type pos = buffer.find_first_of(" \t");

	if (pos != string::npos)
		result = buffer.substr(pos + 1);
	else
		result = buffer;
}


// static
bool POP3Response::checkTerminator(string& buffer, const bool multiLine)
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

	return false;
}


// static
bool POP3Response::checkOneTerminator(string& buffer, const string& term)
{
	if (buffer.length() >= term.length() &&
		std::equal(buffer.end() - term.length(), buffer.end(), term.begin()))
	{
		buffer.erase(buffer.end() - term.length(), buffer.end());
		return true;
	}

	return false;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3
