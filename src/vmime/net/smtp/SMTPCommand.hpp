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

#ifndef VMIME_NET_SMTP_SMTPCOMMAND_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPCOMMAND_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/object.hpp"
#include "vmime/base.hpp"


namespace vmime {


class mailbox;


namespace net {


class socket;
class timeoutHandler;
class tracer;


namespace smtp {


/** A SMTP command, as sent to server.
  */
class VMIME_EXPORT SMTPCommand : public object
{
public:

	static shared_ptr <SMTPCommand> HELO(const string& hostname);
	static shared_ptr <SMTPCommand> EHLO(const string& hostname);
	static shared_ptr <SMTPCommand> AUTH(const string& mechName);
	static shared_ptr <SMTPCommand> AUTH(const string& mechName, const std::string& initialResponse);
	static shared_ptr <SMTPCommand> STARTTLS();
	static shared_ptr <SMTPCommand> MAIL(const mailbox& mbox, const bool utf8);
	static shared_ptr <SMTPCommand> MAIL(const mailbox& mbox, const bool utf8, const size_t size);
	static shared_ptr <SMTPCommand> RCPT(const mailbox& mbox, const bool utf8);
	static shared_ptr <SMTPCommand> RSET();
	static shared_ptr <SMTPCommand> DATA();
	static shared_ptr <SMTPCommand> BDAT(const size_t chunkSize, const bool last);
	static shared_ptr <SMTPCommand> NOOP();
	static shared_ptr <SMTPCommand> QUIT();

	/** Creates a new SMTP command with the specified text.
	  *
	  * @param text command text
	  * @return a new SMTPCommand object
	  */
	static shared_ptr <SMTPCommand> createCommand(const string& text, const string& traceText = "");

	/** Sends this command to the specified socket.
	  *
	  * @param sok socket to which the command will be written
	  * @param tr tracer
	  */
	virtual void writeToSocket(shared_ptr <socket> sok, shared_ptr <tracer> tr);

	/** Returns the full text of the command, including command name
	  * and parameters (if any).
	  *
	  * @return command text (eg. "RCPT TO:<vincent@kisli.com>")
	  */
	virtual const string getText() const;

	/** Returns the full text of the command, suitable for outputing
	  * to the tracer.
	  *
	  * @return trace text (eg. "LOGIN myusername ***")
	  */
	virtual const string getTraceText() const;

protected:

	SMTPCommand(const string& text, const string& traceText);
	SMTPCommand(const SMTPCommand&);

private:

	string m_text;
	string m_traceText;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPCOMMAND_HPP_INCLUDED
