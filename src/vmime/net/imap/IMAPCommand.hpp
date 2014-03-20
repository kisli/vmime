//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2014 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_IMAP_IMAPCOMMAND_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPCOMMAND_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/object.hpp"
#include "vmime/base.hpp"
#include "vmime/dateTime.hpp"

#include "vmime/net/messageSet.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPConnection;


/** An IMAP command that will be sent to the server.
  */
class VMIME_EXPORT IMAPCommand : public object
{
public:

	static shared_ptr <IMAPCommand> LOGIN(const string& username, const string& password);
	static shared_ptr <IMAPCommand> AUTHENTICATE(const string& mechName);
	static shared_ptr <IMAPCommand> AUTHENTICATE(const string& mechName, const string& initialResponse);
	static shared_ptr <IMAPCommand> LIST(const string& refName, const string& mailboxName);
	static shared_ptr <IMAPCommand> SELECT(const bool readOnly, const string& mailboxName, const std::vector <string>& params);
	static shared_ptr <IMAPCommand> STATUS(const string& mailboxName, const std::vector <string>& attribs);
	static shared_ptr <IMAPCommand> CREATE(const string& mailboxName, const std::vector <string>& params);
	static shared_ptr <IMAPCommand> DELETE(const string& mailboxName);
	static shared_ptr <IMAPCommand> RENAME(const string& mailboxName, const string& newMailboxName);
	static shared_ptr <IMAPCommand> FETCH(const messageSet& msgs, const std::vector <string>& params);
	static shared_ptr <IMAPCommand> STORE(const messageSet& msgs, const int mode, const std::vector <string>& flags);
	static shared_ptr <IMAPCommand> APPEND(const string& mailboxName, const std::vector <string>& flags, vmime::datetime* date, const size_t size);
	static shared_ptr <IMAPCommand> COPY(const messageSet& msgs, const string& mailboxName);
	static shared_ptr <IMAPCommand> SEARCH(const std::vector <string>& keys, const vmime::charset* charset);
	static shared_ptr <IMAPCommand> STARTTLS();
	static shared_ptr <IMAPCommand> CAPABILITY();
	static shared_ptr <IMAPCommand> NOOP();
	static shared_ptr <IMAPCommand> EXPUNGE();
	static shared_ptr <IMAPCommand> CLOSE();
	static shared_ptr <IMAPCommand> LOGOUT();

	/** Creates a new IMAP command with the specified text.
	  *
	  * @param text command text
	  * @param traceText trace text (if empty, command text is used)
	  * @return a new IMAPCommand object
	  */
	static shared_ptr <IMAPCommand> createCommand(const string& text, const string& traceText = "");

	/** Sends this command over the specified connection.
	  *
	  * @param conn connection onto which the command will be sent
	  */
	virtual void send(shared_ptr <IMAPConnection> conn);

	/** Returns the full text of the command, including command name
	  * and parameters (if any). This is the text that will be sent
	  * to the server.
	  *
	  * @return command text (eg. "LOGIN myusername mypassword")
	  */
	virtual const string getText() const;

	/** Returns the full text of the command, suitable for outputing
	  * to the tracer.
	  *
	  * @return trace text (eg. "LOGIN {username} {password}")
	  */
	virtual const string getTraceText() const;

protected:

	IMAPCommand(const string& text, const string& traceText);
	IMAPCommand(const IMAPCommand&);

private:

	string m_text;
	string m_traceText;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPCOMMAND_HPP_INCLUDED
