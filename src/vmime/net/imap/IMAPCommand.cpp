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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPCommand.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"

#include <sstream>



namespace vmime {
namespace net {
namespace imap {


IMAPCommand::IMAPCommand(const string& text, const string& traceText)
	: m_text(text), m_traceText(traceText)
{
}


// static
shared_ptr <IMAPCommand> IMAPCommand::LOGIN(const string& username, const string& password)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "LOGIN " << IMAPUtils::quoteString(username)
	    << " " << IMAPUtils::quoteString(password);

	std::ostringstream trace;
	trace.imbue(std::locale::classic());
	trace << "LOGIN {username} {password}";

	return createCommand(cmd.str(), trace.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::AUTHENTICATE(const string& mechName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTHENTICATE " << mechName;

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::AUTHENTICATE(const string& mechName, const string& initialResponse)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "AUTHENTICATE " << mechName << " " << initialResponse;

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::LIST(const string& refName, const string& mailboxName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "LIST " << IMAPUtils::quoteString(refName)
	    << " " << IMAPUtils::quoteString(mailboxName);

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::SELECT
	(const bool readOnly, const string& mailboxName, const std::vector <string>& params)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	if (readOnly)
		cmd << "EXAMINE ";
	else
		cmd << "SELECT ";

	cmd << IMAPUtils::quoteString(mailboxName);

	if (!params.empty())
	{
		cmd << " (";

		for (size_t i = 0, n = params.size() ; i < n ; ++i)
		{
			if (i != 0) cmd << " ";
			cmd << params[i];
		}

		cmd << ")";
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::STATUS
	(const string& mailboxName, const std::vector <string>& attribs)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "STATUS " << IMAPUtils::quoteString(mailboxName);

	cmd << " (";

	for (size_t i = 0, n = attribs.size() ; i < n ; ++i)
	{
		if (i != 0) cmd << " ";
		cmd << attribs[i];
	}

	cmd << ")";

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::CREATE
	(const string& mailboxName, const std::vector <string>& params)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "CREATE " << IMAPUtils::quoteString(mailboxName);

	if (!params.empty())
	{
		cmd << " (";

		for (size_t i = 0, n = params.size() ; i < n ; ++i)
		{
			if (i != 0) cmd << " ";
			cmd << params[i];
		}

		cmd << ")";
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::DELETE(const string& mailboxName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "DELETE " << IMAPUtils::quoteString(mailboxName);

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::RENAME
	(const string& mailboxName, const string& newMailboxName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "RENAME " << IMAPUtils::quoteString(mailboxName)
	    << " " << IMAPUtils::quoteString(newMailboxName);

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::FETCH
	(const messageSet& msgs, const std::vector <string>& params)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	if (msgs.isUIDSet())
		cmd << "UID FETCH " << IMAPUtils::messageSetToSequenceSet(msgs);
	else
		cmd << "FETCH " << IMAPUtils::messageSetToSequenceSet(msgs);

	if (params.size() == 1)
	{
		cmd << " " << params[0];
	}
	else
	{
		cmd << " (";

		for (size_t i = 0, n = params.size() ; i < n ; ++i)
		{
			if (i != 0) cmd << " ";
			cmd << params[i];
		}

		cmd << ")";
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::STORE
	(const messageSet& msgs, const int mode, const std::vector <string>& flags)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	if (msgs.isUIDSet())
		cmd << "UID STORE " << IMAPUtils::messageSetToSequenceSet(msgs);
	else
		cmd << "STORE " << IMAPUtils::messageSetToSequenceSet(msgs);

	if (mode == message::FLAG_MODE_ADD)
		cmd << " +FLAGS ";
	else if (mode == message::FLAG_MODE_REMOVE)
		cmd << " -FLAGS ";
	else // if (mode == message::FLAG_MODE_SET)
		cmd << " FLAGS ";

	cmd << "(";

	for (size_t i = 0, n = flags.size() ; i < n ; ++i)
	{
		if (i != 0) cmd << " ";
		cmd << flags[i];
	}

	cmd << ")";

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::APPEND
	(const string& mailboxName, const std::vector <string>& flags,
	 vmime::datetime* date, const size_t size)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "APPEND " << IMAPUtils::quoteString(mailboxName);

	if (!flags.empty())
	{
		cmd << " (";

		for (size_t i = 0, n = flags.size() ; i < n ; ++i)
		{
			if (i != 0) cmd << " ";
			cmd << flags[i];
		}

		cmd << ")";
	}

	if (date != NULL)
		cmd << " " << IMAPUtils::dateTime(*date);

	cmd << " {" << size << "}";

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::COPY
	(const messageSet& msgs, const string& mailboxName)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());

	if (msgs.isUIDSet())
		cmd << "UID COPY " << IMAPUtils::messageSetToSequenceSet(msgs);
	else
		cmd << "COPY " << IMAPUtils::messageSetToSequenceSet(msgs);

	cmd << " " << IMAPUtils::quoteString(mailboxName);

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::SEARCH
	(const std::vector <string>& keys, const vmime::charset* charset)
{
	std::ostringstream cmd;
	cmd.imbue(std::locale::classic());
	cmd << "SEARCH";

	if (charset)
		cmd << " CHARSET " << charset->getName();

	for (size_t i = 0, n = keys.size() ; i < n ; ++i)
		cmd << " " << keys[i];

	return createCommand(cmd.str());
}


// static
shared_ptr <IMAPCommand> IMAPCommand::STARTTLS()
{
	return createCommand("STARTTLS");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::CAPABILITY()
{
	return createCommand("CAPABILITY");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::NOOP()
{
	return createCommand("NOOP");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::EXPUNGE()
{
	return createCommand("EXPUNGE");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::CLOSE()
{
	return createCommand("CLOSE");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::LOGOUT()
{
	return createCommand("LOGOUT");
}


// static
shared_ptr <IMAPCommand> IMAPCommand::createCommand
	(const string& text, const string& traceText)
{
	if (traceText.empty())
		return shared_ptr <IMAPCommand>(new IMAPCommand(text, text));
	else
		return shared_ptr <IMAPCommand>(new IMAPCommand(text, traceText));
}


const string IMAPCommand::getText() const
{
	return m_text;
}


const string IMAPCommand::getTraceText() const
{
	return m_traceText;
}


void IMAPCommand::send(shared_ptr <IMAPConnection> conn)
{
	conn->sendCommand(dynamicCast <IMAPCommand>(shared_from_this()));
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP
