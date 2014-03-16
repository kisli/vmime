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


#include "vmime/net/pop3/POP3Command.hpp"
#include "vmime/net/pop3/POP3Connection.hpp"
#include "vmime/net/pop3/POP3Store.hpp"

#include "vmime/net/socket.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3Command::POP3Command(const string& text, const string& traceText)
	: m_text(text), m_traceText(traceText)
{
}


// static
shared_ptr <POP3Command> POP3Command::CAPA()
{
	return createCommand("CAPA");
}


// static
shared_ptr <POP3Command> POP3Command::NOOP()
{
	return createCommand("NOOP");
}


// static
shared_ptr <POP3Command> POP3Command::AUTH(const string& mechName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTH " << mechName;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::AUTH(const string& mechName, const string& initialResponse)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTH " << mechName << " " << initialResponse;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::STLS()
{
	return createCommand("STLS");
}


// static
shared_ptr <POP3Command> POP3Command::APOP(const string& username, const string& digest)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "APOP " << username << " " << digest;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::USER(const string& username)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "USER " << username;

	std::ostringstream trace;
	trace.imbue(std::locale::classic());
	trace << "USER {username}";

	return createCommand(cmd.str(), trace.str());
}


// static
shared_ptr <POP3Command> POP3Command::PASS(const string& password)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "PASS " << password;

	std::ostringstream trace;
	trace.imbue(std::locale::classic());
	trace << "PASS {password}";

	return createCommand(cmd.str(), trace.str());
}


// static
shared_ptr <POP3Command> POP3Command::STAT()
{
	return createCommand("STAT");
}


// static
shared_ptr <POP3Command> POP3Command::LIST()
{
	return createCommand("LIST");
}


// static
shared_ptr <POP3Command> POP3Command::LIST(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "LIST " << msg;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::UIDL()
{
	return createCommand("UIDL");
}


// static
shared_ptr <POP3Command> POP3Command::UIDL(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "UIDL " << msg;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::DELE(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "DELE " << msg;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::RETR(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "RETR " << msg;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::TOP(const unsigned long msg, const unsigned long lines)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "TOP " << msg << " " << lines;

	return createCommand(cmd.str());
}


// static
shared_ptr <POP3Command> POP3Command::RSET()
{
	return createCommand("RSET");
}


// static
shared_ptr <POP3Command> POP3Command::QUIT()
{
	return createCommand("QUIT");
}


// static
shared_ptr <POP3Command> POP3Command::createCommand
	(const string& text, const string& traceText)
{
	if (traceText.empty())
		return shared_ptr <POP3Command>(new POP3Command(text, text));
	else
		return shared_ptr <POP3Command>(new POP3Command(text, traceText));
}


const string POP3Command::getText() const
{
	return m_text;
}


const string POP3Command::getTraceText() const
{
	return m_traceText;
}


void POP3Command::send(shared_ptr <POP3Connection> conn)
{
	conn->getSocket()->send(m_text + "\r\n");

	if (conn->getTracer())
		conn->getTracer()->traceSend(m_traceText);
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3
