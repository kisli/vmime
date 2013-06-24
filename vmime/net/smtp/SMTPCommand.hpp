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


namespace smtp {


/** A SMTP command, as sent to server.
  */
class VMIME_EXPORT SMTPCommand : public object
{
	friend class vmime::creator;

public:

	static ref <SMTPCommand> HELO(const string& hostname);
	static ref <SMTPCommand> EHLO(const string& hostname);
	static ref <SMTPCommand> AUTH(const string& mechName);
	static ref <SMTPCommand> STARTTLS();
	static ref <SMTPCommand> MAIL(const mailbox& mbox, const bool utf8);
	static ref <SMTPCommand> MAIL(const mailbox& mbox, const bool utf8, const unsigned long size);
	static ref <SMTPCommand> RCPT(const mailbox& mbox, const bool utf8);
	static ref <SMTPCommand> RSET();
	static ref <SMTPCommand> DATA();
	static ref <SMTPCommand> BDAT(const unsigned long chunkSize, const bool last);
	static ref <SMTPCommand> NOOP();
	static ref <SMTPCommand> QUIT();

	/** Creates a new SMTP command with the specified text.
	  *
	  * @param text command text
	  * @return a new SMTPCommand object
	  */
	static ref <SMTPCommand> createCommand(const string& text);

	/** Sends this command to the specified socket.
	  *
	  * @param sok socket to which the command will be written
	  */
	virtual void writeToSocket(ref <socket> sok);

	/** Returns the full text of the command, including command name
	  * and parameters (if any).
	  *
	  * @return command text (eg. "RCPT TO:<vincent@kisli.com>")
	  */
	virtual const string getText() const;

protected:

	SMTPCommand(const string& text);
	SMTPCommand(const SMTPCommand&);

private:

	string m_text;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPCOMMAND_HPP_INCLUDED
