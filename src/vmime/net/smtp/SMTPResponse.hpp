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

#ifndef VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/object.hpp"
#include "vmime/base.hpp"


namespace vmime {
namespace net {


class socket;
class timeoutHandler;
class tracer;


namespace smtp {


/** A SMTP response, as sent by the server.
  */
class VMIME_EXPORT SMTPResponse : public object
{
public:

	/** Current state of response parser. */
	struct state
	{
		string responseBuffer;
	};

	/** Enhanced status code (as per RFC-3463). */
	struct enhancedStatusCode
	{
		enhancedStatusCode();
		enhancedStatusCode(const enhancedStatusCode& enhCode);

		unsigned short klass;     /**< Success/failure. */
		unsigned short subject;   /**< Source of anomaly. */
		unsigned short detail;    /**< Precise error condition. */
	};

	/** An element of a SMTP response. */
	class responseLine
	{
	public:

		responseLine(const int code, const string& text, const enhancedStatusCode& enhCode);

		void setCode(const int code);
		int getCode() const;

		void setEnhancedCode(const enhancedStatusCode& enhCode);
		const enhancedStatusCode getEnhancedCode() const;

		void setText(const string& text);
		const string getText() const;

	private:

		int m_code;
		string m_text;
		enhancedStatusCode m_enhCode;
	};

	/** Receive and parse a new SMTP response from the
	  * specified socket.
	  *
	  * @param tr tracer
	  * @param sok socket from which to read
	  * @param toh time-out handler
	  * @param st previous state of response parser for the specified socket
	  * @return SMTP response
	  * @throws exceptions::operation_timed_out if no data
	  * has been received within the granted time
	  */
	static shared_ptr <SMTPResponse> readResponse
		(shared_ptr <tracer> tr, shared_ptr <socket> sok,
		 shared_ptr <timeoutHandler> toh, const state& st);

	/** Return the SMTP response code.
	  *
	  * @return response code
	  */
	int getCode() const;

	/** Return the SMTP enhanced status code, if available.
	  *
	  * @return enhanced status code
	  */
	const enhancedStatusCode getEnhancedCode() const;

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
	const responseLine getLineAt(const size_t pos) const;

	/** Return the number of lines in the response.
	  *
	  * @return number of lines in the response
	  */
	size_t getLineCount() const;

	/** Return the last line in the response.
	  *
	  * @return last response line
	  */
	const responseLine getLastLine() const;

	/** Returns the current state of the response parser.
	  *
	  * @return current parser state
	  */
	const state getCurrentState() const;

private:

	SMTPResponse(shared_ptr <tracer> tr, shared_ptr <socket> sok, shared_ptr <timeoutHandler> toh, const state& st);
	SMTPResponse(const SMTPResponse&);

	void readResponse();

	const string readResponseLine();
	const responseLine getNextResponse();

	static int extractResponseCode(const string& response);
	static const enhancedStatusCode extractEnhancedCode(const string& responseText);


	std::vector <responseLine> m_lines;

	shared_ptr <socket> m_socket;
	shared_ptr <timeoutHandler> m_timeoutHandler;
	shared_ptr <tracer> m_tracer;

	string m_responseBuffer;
	bool m_responseContinues;
};


VMIME_EXPORT std::ostream& operator<<(std::ostream& os, const SMTPResponse::enhancedStatusCode& code);


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPRESPONSE_HPP_INCLUDED

