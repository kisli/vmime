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

#ifndef VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED


#include "vmime/object.hpp"
#include "vmime/base.hpp"


namespace vmime {
namespace net {


class socket;
class timeoutHandler;


namespace smtp {


/** A SMTP response, as sent by the server.
  */
class SMTPResponse : public object
{
	friend class vmime::creator;

public:

	/** An element of a SMTP response. */
	class responseLine
	{
	public:

		responseLine(const int code, const string& text);

		void setCode(const int code);
		int getCode() const;

		void setText(const string& text);
		const string getText() const;

	private:

		int m_code;
		string m_text;
	};

	/** Receive and parse a new SMTP response from the
	  * specified socket.
	  *
	  * @param sok socket from which to read
	  * @param toh time-out handler
	  * @return SMTP response
	  * @throws exceptions::operation_timed_out if no data
	  * has been received within the granted time
	  */
	static ref <SMTPResponse> readResponse(ref <socket> sok, ref <timeoutHandler> toh);

	/** Return the SMTP response code.
	  *
	  * @return response code
	  */
	int getCode() const;

	/** Return the SMTP response text.
	  * The text of each line is concatenated.
	  *
	  * @return response text
	  */
	const string getText() const;

	/** Return the response line at the specified position.
	  *
	  * @param pos line index
	  * @return line at the specified index
	  */
	const responseLine getLineAt(const unsigned int pos) const;

	/** Return the number of lines in the response.
	  *
	  * @return number of lines in the response
	  */
	unsigned int getLineCount() const;

	/** Return the last line in the response.
	  *
	  * @return last response line
	  */
	const responseLine getLastLine() const;

private:

	SMTPResponse(ref <socket> sok, ref <timeoutHandler> toh);
	SMTPResponse(const SMTPResponse&);

	void readResponse();

	const string readResponseLine();
	const responseLine getNextResponse();

	static int extractResponseCode(const string& response);


	std::vector <responseLine> m_lines;

	ref <socket> m_socket;
	ref <timeoutHandler> m_timeoutHandler;

	string m_responseBuffer;
	bool m_responseContinues;
};


} // smtp
} // net
} // vmime


#endif // VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED

