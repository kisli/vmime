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

#include "vmime/net/socket.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3Command::POP3Command(const string& text)
	: m_text(text)
{
}


// static
ref <POP3Command> POP3Command::CAPA()
{
	return createCommand("CAPA");
}


// static
ref <POP3Command> POP3Command::NOOP()
{
	return createCommand("NOOP");
}


// static
ref <POP3Command> POP3Command::AUTH(const string& mechName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTH " << mechName;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::STLS()
{
	return createCommand("STLS");
}


// static
ref <POP3Command> POP3Command::APOP(const string& username, const string& digest)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "APOP " << username << " " << digest;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::USER(const string& username)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "USER " << username;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::PASS(const string& password)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "PASS " << password;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::STAT()
{
	return createCommand("STAT");
}


// static
ref <POP3Command> POP3Command::LIST()
{
	return createCommand("LIST");
}


// static
ref <POP3Command> POP3Command::LIST(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "LIST " << msg;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::UIDL()
{
	return createCommand("UIDL");
}


// static
ref <POP3Command> POP3Command::UIDL(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "UIDL " << msg;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::DELE(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "DELE " << msg;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::RETR(const unsigned long msg)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "RETR " << msg;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::TOP(const unsigned long msg, const unsigned long lines)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "TOP " << msg << " " << lines;

	return createCommand(cmd.str());
}


// static
ref <POP3Command> POP3Command::RSET()
{
	return createCommand("RSET");
}


// static
ref <POP3Command> POP3Command::QUIT()
{
	return createCommand("QUIT");
}


// static
ref <POP3Command> POP3Command::createCommand(const std::string& text)
{
	return vmime::create <POP3Command>(text);
}


const string POP3Command::getText() const
{
	return m_text;
}


void POP3Command::send(ref <POP3Connection> conn)
{
	conn->getSocket()->send(m_text + "\r\n");
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3
