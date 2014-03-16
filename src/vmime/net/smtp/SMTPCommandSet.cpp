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


#include "vmime/net/smtp/SMTPCommandSet.hpp"

#include "vmime/net/socket.hpp"

#include "vmime/mailbox.hpp"

#include <stdexcept>


namespace vmime {
namespace net {
namespace smtp {


SMTPCommandSet::SMTPCommandSet(const bool pipeline)
	: SMTPCommand("", ""), m_pipeline(pipeline),
	  m_started(false), m_lastCommandSent()
{
}


// static
shared_ptr <SMTPCommandSet> SMTPCommandSet::create(const bool pipeline)
{
	return shared_ptr <SMTPCommandSet>(new SMTPCommandSet(pipeline));
}


void SMTPCommandSet::addCommand(shared_ptr <SMTPCommand> cmd)
{
	if (m_started)
	{
		throw std::runtime_error("Could not add command to pipeline: "
			"one or more commands have already been sent to the server.");
	}

	m_commands.push_back(cmd);
}


void SMTPCommandSet::writeToSocket(shared_ptr <socket> sok, shared_ptr <tracer> tr)
{
	if (m_pipeline)
	{
		if (!m_started)
		{
			// Send all commands at once
			for (std::list <shared_ptr <SMTPCommand> >::const_iterator it = m_commands.begin() ;
			     it != m_commands.end() ; ++it)
			{
				shared_ptr <SMTPCommand> cmd = *it;
				cmd->writeToSocket(sok, tr);
			}
		}

		if (!m_commands.empty())
		{
			// Advance the pointer to last command sent
			shared_ptr <SMTPCommand> cmd = m_commands.front();
			m_commands.pop_front();

			m_lastCommandSent = cmd;
		}
	}
	else
	{
		if (!m_commands.empty())
		{
			// Send only one command
			shared_ptr <SMTPCommand> cmd = m_commands.front();
			m_commands.pop_front();

			cmd->writeToSocket(sok, tr);

			m_lastCommandSent = cmd;
		}
	}

	m_started = true;
}


const string SMTPCommandSet::getText() const
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	for (std::list <shared_ptr <SMTPCommand> >::const_iterator it = m_commands.begin() ;
	     it != m_commands.end() ; ++it)
	{
		cmd << (*it)->getText() << "\r\n";
	}

	return cmd.str();
}


const string SMTPCommandSet::getTraceText() const
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	for (std::list <shared_ptr <SMTPCommand> >::const_iterator it = m_commands.begin() ;
	     it != m_commands.end() ; ++it)
	{
		cmd << (*it)->getTraceText() << "\r\n";
	}

	return cmd.str();
}


bool SMTPCommandSet::isFinished() const
{
	return (m_pipeline && m_started) || (m_commands.size() == 0 && m_started);
}


shared_ptr <SMTPCommand> SMTPCommandSet::getLastCommandSent() const
{
	return m_lastCommandSent;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
