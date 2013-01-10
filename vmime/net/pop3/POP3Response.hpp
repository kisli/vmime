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

#ifndef VMIME_NET_SMTP_POP3RESPONSE_HPP_INCLUDED
#define VMIME_NET_SMTP_POP3RESPONSE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/object.hpp"
#include "vmime/base.hpp"

#include "vmime/utility/outputStream.hpp"
#include "vmime/utility/progressListener.hpp"

#include "vmime/net/socket.hpp"


namespace vmime {
namespace net {


class timeoutHandler;


namespace pop3 {


/** A POP3 response, as sent by the server.
  */
class POP3Response : public object
{
	friend class vmime::creator;

public:

	/** Possible response codes. */
	enum ResponseCode
	{
		CODE_OK = 0,
		CODE_READY,
		CODE_ERR
	};


	/** Receive and parse a POP3 response from the
	  * specified socket.
	  *
	  * @param sok socket from which to read
	  * @param toh time-out handler (can be NULL)
	  * @return POP3 response
	  * @throws exceptions::operation_timed_out if no data
	  * has been received within the granted time
	  */
	static ref <POP3Response> readResponse
		(ref <socket> sok, ref <timeoutHandler> toh);

	/** Receive and parse a multiline POP3 response from
	  * the specified socket.
	  *
	  * @param sok socket from which to read
	  * @param toh time-out handler (can be NULL)
	  * @return POP3 response
	  * @throws exceptions::operation_timed_out if no data
	  * has been received within the granted time
	  */
	static ref <POP3Response> readMultilineResponse
		(ref <socket> sok, ref <timeoutHandler> toh);

	/** Receive and parse a large POP3 response (eg. message data)
	  * from the specified socket.
	  *
	  * @param sok socket from which to read
	  * @param toh time-out handler (can be NULL)
	  * @param os output stream to which response data will be written
	  * @param progress progress listener (can be NULL)
	  * @param predictedSize estimated size of response data (in bytes)
	  * @return POP3 response
	  * @throws exceptions::operation_timed_out if no data
	  * has been received within the granted time
	  */
	static ref <POP3Response> readLargeResponse
		(ref <socket> sok, ref <timeoutHandler> toh,
		 utility::outputStream& os,
		 utility::progressListener* progress, const long predictedSize);


	/** Returns whether the response is successful ("OK").
	  *
	  * @return true if the response if successful, false otherwise
	  */
	bool isSuccess() const;

	/** Return the POP3 response code.
	  *
	  * @return response code
	  */
	ResponseCode getCode() const;

	/** Return the POP3 response text (first line).
	  *
	  * @return response text
	  */
	const string getText() const;

	/** Return the first POP3 response line.
	  *
	  * @return first response line
	  */
	const string getFirstLine() const;

	/** Return the response line at the specified position.
	  *
	  * @param pos line index
	  * @return line at the specified index
	  */
	const string getLineAt(const size_t pos) const;

	/** Return the number of lines in the response.
	  *
	  * @return number of lines in the response
	  */
	size_t getLineCount() const;

private:

	POP3Response(ref <socket> sok, ref <timeoutHandler> toh);

	void readResponseImpl(string& buffer, const bool multiLine);
	void readResponseImpl
		(string& firstLine, utility::outputStream& os,
		 utility::progressListener* progress, const long predictedSize);


	static bool stripFirstLine(const string& buffer, string& result, string* firstLine);

	static ResponseCode getResponseCode(const string& buffer);

	static void stripResponseCode(const string& buffer, string& result);

	static bool checkTerminator(string& buffer, const bool multiLine);
	static bool checkOneTerminator(string& buffer, const string& term);


	ref <socket> m_socket;
	ref <timeoutHandler> m_timeoutHandler;

	string m_firstLine;
	ResponseCode m_code;
	string m_text;

	std::vector <string> m_lines;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_SMTP_POP3RESPONSE_HPP_INCLUDED
