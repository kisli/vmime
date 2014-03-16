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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPResponse.hpp"

#include "vmime/platform.hpp"
#include "vmime/utility/stringUtils.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/tracer.hpp"

#include <cctype>


namespace vmime {
namespace net {
namespace smtp {


SMTPResponse::SMTPResponse
	(shared_ptr <tracer> tr, shared_ptr <socket> sok,
	 shared_ptr <timeoutHandler> toh, const state& st)
	: m_socket(sok), m_timeoutHandler(toh), m_tracer(tr),
	  m_responseBuffer(st.responseBuffer), m_responseContinues(false)
{
}


SMTPResponse::SMTPResponse(const SMTPResponse&)
	: vmime::object()
{
	// Not used
}


int SMTPResponse::getCode() const
{
	const int firstCode = m_lines[0].getCode();

	for (unsigned int i = 1 ; i < m_lines.size() ; ++i)
	{
		// All response codes returned must be equal
		// or else this in an error...
		if (m_lines[i].getCode() != firstCode)
			return 0;
	}

	return firstCode;
}


const SMTPResponse::enhancedStatusCode SMTPResponse::getEnhancedCode() const
{
	return m_lines[m_lines.size() - 1].getEnhancedCode();
}


const string SMTPResponse::getText() const
{
	string text = m_lines[0].getText();

	for (unsigned int i = 1 ; i < m_lines.size() ; ++i)
	{
		text += '\n';
		text += m_lines[i].getText();
	}

	return text;
}


// static
shared_ptr <SMTPResponse> SMTPResponse::readResponse
	(shared_ptr <tracer> tr, shared_ptr <socket> sok,
	 shared_ptr <timeoutHandler> toh, const state& st)
{
	shared_ptr <SMTPResponse> resp =
		shared_ptr <SMTPResponse>(new SMTPResponse(tr, sok, toh, st));

	resp->readResponse();

	return resp;
}


void SMTPResponse::readResponse()
{
	responseLine line = getNextResponse();
	m_lines.push_back(line);

	while (m_responseContinues)
	{
		line = getNextResponse();
		m_lines.push_back(line);
	}
}


const string SMTPResponse::readResponseLine()
{
	string currentBuffer = m_responseBuffer;

	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	while (true)
	{
		// Get a line from the response buffer
		const size_t lineEnd = currentBuffer.find_first_of('\n');

		if (lineEnd != string::npos)
		{
			size_t actualLineEnd = lineEnd;

			if (actualLineEnd != 0 && currentBuffer[actualLineEnd - 1] == '\r')  // CRLF case
				actualLineEnd--;

			const string line(currentBuffer.begin(), currentBuffer.begin() + actualLineEnd);

			currentBuffer.erase(currentBuffer.begin(), currentBuffer.begin() + lineEnd + 1);
			m_responseBuffer = currentBuffer;

			if (m_tracer)
				m_tracer->traceReceive(line);

			return line;
		}

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
			m_socket->waitForRead();
			continue;
		}

		currentBuffer += receiveBuffer;
	}
}


const SMTPResponse::responseLine SMTPResponse::getNextResponse()
{
	string line = readResponseLine();

	const int code = extractResponseCode(line);
	string text;

	m_responseContinues = (line.length() >= 4 && line[3] == '-');

	if (line.length() > 4)
		text = utility::stringUtils::trim(line.substr(4));
	else
		text = "";

	return responseLine(code, text, extractEnhancedCode(text));
}


// static
int SMTPResponse::extractResponseCode(const string& response)
{
	int code = 0;

	if (response.length() >= 3)
	{
		code = (response[0] - '0') * 100
		     + (response[1] - '0') * 10
		     + (response[2] - '0');
	}

	return code;
}


// static
const SMTPResponse::enhancedStatusCode SMTPResponse::extractEnhancedCode(const string& responseText)
{
	enhancedStatusCode enhCode;

	std::istringstream iss(responseText);

	if (std::isdigit(iss.peek()))
	{
		iss >> enhCode.klass;

		if (iss.get() == '.' && std::isdigit(iss.peek()))
		{
			iss >> enhCode.subject;

			if (iss.get() == '.' && std::isdigit(iss.peek()))
			{
				iss >> enhCode.detail;
				return enhCode;
			}
		}
	}

	return enhancedStatusCode();   // no enhanced code found
}


const SMTPResponse::responseLine SMTPResponse::getLineAt(const size_t pos) const
{
	return m_lines[pos];
}


size_t SMTPResponse::getLineCount() const
{
	return m_lines.size();
}


const SMTPResponse::responseLine SMTPResponse::getLastLine() const
{
	return m_lines[m_lines.size() - 1];
}


const SMTPResponse::state SMTPResponse::getCurrentState() const
{
	state st;
	st.responseBuffer = m_responseBuffer;

	return st;
}



// SMTPResponse::responseLine

SMTPResponse::responseLine::responseLine(const int code, const string& text, const enhancedStatusCode& enhCode)
	: m_code(code), m_text(text), m_enhCode(enhCode)
{
}


void SMTPResponse::responseLine::setCode(const int code)
{
	m_code = code;
}


int SMTPResponse::responseLine::getCode() const
{
	return m_code;
}


void SMTPResponse::responseLine::setEnhancedCode(const enhancedStatusCode& enhCode)
{
	m_enhCode = enhCode;
}


const SMTPResponse::enhancedStatusCode SMTPResponse::responseLine::getEnhancedCode() const
{
	return m_enhCode;
}


void SMTPResponse::responseLine::setText(const string& text)
{
	m_text = text;
}


const string SMTPResponse::responseLine::getText() const
{
	return m_text;
}



// SMTPResponse::enhancedStatusCode


SMTPResponse::enhancedStatusCode::enhancedStatusCode()
	: klass(0), subject(0), detail(0)
{
}


SMTPResponse::enhancedStatusCode::enhancedStatusCode(const enhancedStatusCode& enhCode)
	: klass(enhCode.klass), subject(enhCode.subject), detail(enhCode.detail)
{
}


std::ostream& operator<<(std::ostream& os, const SMTPResponse::enhancedStatusCode& code)
{
	os << code.klass << '.' << code.subject << '.' << code.detail;
	return os;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

