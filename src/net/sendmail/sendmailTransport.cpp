//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/net/sendmail/sendmailTransport.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"
#include "vmime/message.hpp"
#include "vmime/mailboxList.hpp"

#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/childProcess.hpp"
#include "vmime/utility/smartPtr.hpp"

#include "vmime/config.hpp"


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(sm_infos.getPropertyValue <type>(getSession(), sm_infos.getProperties().prop))
#define HAS_PROPERTY(prop) \
	(sm_infos.hasProperty(getSession(), sm_infos.getProperties().prop))


#if VMIME_BUILTIN_PLATFORM_POSIX


namespace vmime {
namespace net {
namespace sendmail {


sendmailTransport::sendmailTransport(ref <session> sess, ref <security::authenticator> auth)
	: transport(sess, getInfosInstance(), auth), m_connected(false)
{
}


sendmailTransport::~sendmailTransport()
{
	try
	{
		if (isConnected())
			disconnect();
	}
	catch (vmime::exception&)
	{
		// Ignore
	}
}


const string sendmailTransport::getProtocolName() const
{
	return "sendmail";
}


void sendmailTransport::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	// Use the specified path for 'sendmail' or a default one if no path is specified
	m_sendmailPath = GET_PROPERTY(string, PROPERTY_BINPATH);

	m_connected = true;
}


const bool sendmailTransport::isConnected() const
{
	return (m_connected);
}


void sendmailTransport::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void sendmailTransport::internalDisconnect()
{
	m_connected = false;
}


void sendmailTransport::noop()
{
	// Do nothing
}


void sendmailTransport::send
	(const mailbox& expeditor, const mailboxList& recipients,
	 utility::inputStream& is, const utility::stream::size_type size,
         utility::progressionListener* progress)
{
	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty())
		throw exceptions::no_recipient();
	else if (expeditor.isEmpty())
		throw exceptions::no_expeditor();

	// Construct the argument list
	std::vector <string> args;

	args.push_back("-i");
	args.push_back("-f");
	args.push_back(expeditor.getEmail());
	args.push_back("--");

	for (int i = 0 ; i < recipients.getMailboxCount() ; ++i)
		args.push_back(recipients.getMailboxAt(i)->getEmail());

	// Call sendmail
	try
	{
		internalSend(args, is, size, progress);
	}
	catch (vmime::exception& e)
	{
		throw exceptions::command_error("SEND", "", "sendmail failed", e);
	}
}


void sendmailTransport::internalSend
	(const std::vector <string> args, utility::inputStream& is,
	 const utility::stream::size_type size, utility::progressionListener* progress)
{
	const utility::file::path path = vmime::platformDependant::getHandler()->
		getFileSystemFactory()->stringToPath(m_sendmailPath);

	ref <utility::childProcess> proc =
		vmime::platformDependant::getHandler()->
			getChildProcessFactory()->create(path);

	proc->start(args, utility::childProcess::FLAG_REDIRECT_STDIN);

	// Copy message data from input stream to output pipe
	utility::outputStream& os = *(proc->getStdIn());

	// Workaround for lame sendmail implementations that
	// can't handle CRLF eoln sequences: we transform CRLF
	// sequences into LF characters.
	utility::CRLFToLFFilteredOutputStream fos(os);

	// TODO: remove 'Bcc:' field from message header

	utility::bufferedStreamCopy(is, fos, size, progress);

	// Wait for sendmail to exit
	proc->waitForFinish();
}


// Service infos

sendmailTransport::_infos sendmailTransport::sm_infos;


const serviceInfos& sendmailTransport::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& sendmailTransport::getInfos() const
{
	return (sm_infos);
}


const string sendmailTransport::_infos::getPropertyPrefix() const
{
	return "transport.sendmail.";
}


const sendmailTransport::_infos::props& sendmailTransport::_infos::getProperties() const
{
	static props p =
	{
		// Path to sendmail (override default)
		property("binpath", serviceInfos::property::TYPE_STRING, string(VMIME_SENDMAIL_PATH))
	};

	return p;
}


const std::vector <serviceInfos::property> sendmailTransport::_infos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	list.push_back(p.PROPERTY_BINPATH);

	return (list);
}


} // sendmail
} // net
} // vmime


#endif // VMIME_BUILTIN_PLATFORM_POSIX
