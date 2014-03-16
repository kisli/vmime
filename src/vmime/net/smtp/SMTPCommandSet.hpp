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

#ifndef VMIME_NET_SMTP_SMTPCOMMANDSET_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPCOMMANDSET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include <list>

#include "vmime/net/smtp/SMTPCommand.hpp"


namespace vmime {
namespace net {
namespace smtp {


/** A set of SMTP commands, which may be sent all at once
  * to the server if pipelining is supported.
  */
class VMIME_EXPORT SMTPCommandSet : public SMTPCommand
{
public:

	/** Creates a new set of SMTP commands.
	  *
	  * @param pipeline set to true if the server supports pipelining
	  * @return a new SMTPCommandSet object
	  */
	static shared_ptr <SMTPCommandSet> create(const bool pipeline);

	/** Adds a new command to this set.
	  * If one or more comments have already been sent to the server,
	  * an exception will be thrown.
	  *
	  * @param cmd command to add
	  */
	void addCommand(shared_ptr <SMTPCommand> cmd);

	/** Tests whether all commands have been sent.
	  *
	  * @return true if all commands have been sent,
	  * or false otherwise
	  */
	bool isFinished() const;

	/** Returns the last command which has been sent.
	  *
	  * @return a pointer to a SMTPCommand, of NULL if no command
	  * has been sent yet
	  */
	shared_ptr <SMTPCommand> getLastCommandSent() const;


	void writeToSocket(shared_ptr <socket> sok, shared_ptr <tracer> tr);

	const string getText() const;
	const string getTraceText() const;

private:

	SMTPCommandSet(const bool pipeline);
	SMTPCommandSet(const SMTPCommandSet&);


	bool m_pipeline;
	bool m_started;
	std::list <shared_ptr <SMTPCommand> > m_commands;
	shared_ptr <SMTPCommand> m_lastCommandSent;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPCOMMANDSET_HPP_INCLUDED
