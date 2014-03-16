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

#ifndef VMIME_NET_POP3_POP3COMMAND_HPP_INCLUDED
#define VMIME_NET_POP3_POP3COMMAND_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/object.hpp"
#include "vmime/base.hpp"


namespace vmime {


class mailbox;


namespace net {
namespace pop3 {


class POP3Connection;


/** A POP3 command that will be sent to the server.
  */
class VMIME_EXPORT POP3Command : public object
{
public:

	static shared_ptr <POP3Command> CAPA();
	static shared_ptr <POP3Command> NOOP();
	static shared_ptr <POP3Command> AUTH(const string& mechName);
	static shared_ptr <POP3Command> AUTH(const string& mechName, const string& initialResponse);
	static shared_ptr <POP3Command> STLS();
	static shared_ptr <POP3Command> APOP(const string& username, const string& digest);
	static shared_ptr <POP3Command> USER(const string& username);
	static shared_ptr <POP3Command> PASS(const string& password);
	static shared_ptr <POP3Command> STAT();
	static shared_ptr <POP3Command> LIST();
	static shared_ptr <POP3Command> LIST(const unsigned long msg);
	static shared_ptr <POP3Command> UIDL();
	static shared_ptr <POP3Command> UIDL(const unsigned long msg);
	static shared_ptr <POP3Command> DELE(const unsigned long msg);
	static shared_ptr <POP3Command> RETR(const unsigned long msg);
	static shared_ptr <POP3Command> TOP(const unsigned long msg, const unsigned long lines);
	static shared_ptr <POP3Command> RSET();
	static shared_ptr <POP3Command> QUIT();

	/** Creates a new POP3 command with the specified text.
	  *
	  * @param text command text
	  * @param traceText trace text (if empty, command text is used)
	  * @return a new POP3Command object
	  */
	static shared_ptr <POP3Command> createCommand(const string& text, const string& traceText = "");

	/** Sends this command over the specified connection.
	  *
	  * @param conn connection onto which the command will be sent
	  */
	virtual void send(shared_ptr <POP3Connection> conn);

	/** Returns the full text of the command, including command name
	  * and parameters (if any).
	  *
	  * @return command text (eg. "LIST 42")
	  */
	virtual const string getText() const;

	/** Returns the full text of the command, suitable for outputing
	  * to the tracer.
	  *
	  * @return trace text (eg. "USER myusername")
	  */
	virtual const string getTraceText() const;

protected:

	POP3Command(const string& text, const string& traceText);
	POP3Command(const POP3Command&);

private:

	string m_text;
	string m_traceText;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3COMMAND_HPP_INCLUDED
