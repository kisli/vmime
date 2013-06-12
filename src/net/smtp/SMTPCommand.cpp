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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPCommand.hpp"

#include "vmime/net/socket.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"


namespace vmime {
namespace net {
namespace smtp {


SMTPCommand::SMTPCommand(const string& text)
	: m_text(text)
{
}


// static
ref <SMTPCommand> SMTPCommand::EHLO(const string& hostname)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "EHLO " << hostname;

	return createCommand(cmd.str());
}


// static
ref <SMTPCommand> SMTPCommand::HELO(const string& hostname)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "HELO " << hostname;

	return createCommand(cmd.str());
}


// static
ref <SMTPCommand> SMTPCommand::AUTH(const string& mechName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTH " << mechName;

	return createCommand(cmd.str());
}


// static
ref <SMTPCommand> SMTPCommand::STARTTLS()
{
	return createCommand("STARTTLS");
}


// static
ref <SMTPCommand> SMTPCommand::MAIL(const mailbox& mbox, const bool utf8)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "MAIL FROM:<";

	if (utf8)
	{
		cmd << mbox.getEmail().toText().getConvertedText(vmime::charsets::UTF_8);
	}
	else
	{
		vmime::utility::outputStreamAdapter cmd2(cmd);
		mbox.getEmail().generate(cmd2);
	}

	cmd << ">";

	if (utf8)
		cmd << " SMTPUTF8";

	return createCommand(cmd.str());
}


// static
ref <SMTPCommand> SMTPCommand::RCPT(const mailbox& mbox, const bool utf8)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "RCPT TO:<";

	if (utf8)
	{
		cmd << mbox.getEmail().toText().getConvertedText(vmime::charsets::UTF_8);
	}
	else
	{
		vmime::utility::outputStreamAdapter cmd2(cmd);
		mbox.getEmail().generate(cmd2);
	}

	cmd << ">";

	return createCommand(cmd.str());
}


// static
ref <SMTPCommand> SMTPCommand::RSET()
{
	return createCommand("RSET");
}


// static
ref <SMTPCommand> SMTPCommand::DATA()
{
	return createCommand("DATA");
}


// static
ref <SMTPCommand> SMTPCommand::NOOP()
{
	return createCommand("NOOP");
}


// static
ref <SMTPCommand> SMTPCommand::QUIT()
{
	return createCommand("QUIT");
}


// static
ref <SMTPCommand> SMTPCommand::createCommand(const string& text)
{
	return vmime::create <SMTPCommand>(text);
}


const string SMTPCommand::getText() const
{
	return m_text;
}


void SMTPCommand::writeToSocket(ref <socket> sok)
{
	sok->send(m_text + "\r\n");
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
