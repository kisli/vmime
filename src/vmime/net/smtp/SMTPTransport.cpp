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


#include "vmime/net/smtp/SMTPTransport.hpp"
#include "vmime/net/smtp/SMTPResponse.hpp"
#include "vmime/net/smtp/SMTPCommand.hpp"
#include "vmime/net/smtp/SMTPCommandSet.hpp"
#include "vmime/net/smtp/SMTPChunkingOutputStreamAdapter.hpp"
#include "vmime/net/smtp/SMTPExceptions.hpp"

#include "vmime/exception.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/message.hpp"

#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/outputStreamSocketAdapter.hpp"
#include "vmime/utility/streamUtils.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace net {
namespace smtp {


SMTPTransport::SMTPTransport(shared_ptr <session> sess, shared_ptr <security::authenticator> auth, const bool secured)
	: transport(sess, getInfosInstance(), auth), m_isSMTPS(secured), m_needReset(false)
{
}


SMTPTransport::~SMTPTransport()
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


const string SMTPTransport::getProtocolName() const
{
	return "smtp";
}


bool SMTPTransport::isSMTPS() const
{
	return m_isSMTPS;
}


void SMTPTransport::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_connection = make_shared <SMTPConnection>
		(dynamicCast <SMTPTransport>(shared_from_this()), getAuthenticator());

	try
	{
		m_connection->connect();
	}
	catch (std::exception&)
	{
		m_connection = null;
		throw;
	}
}


bool SMTPTransport::isConnected() const
{
	return m_connection && m_connection->isConnected();
}


bool SMTPTransport::isSecuredConnection() const
{
	if (m_connection == NULL)
		return false;

	return m_connection->isSecuredConnection();
}


shared_ptr <connectionInfos> SMTPTransport::getConnectionInfos() const
{
	if (m_connection == NULL)
		return null;

	return m_connection->getConnectionInfos();
}


shared_ptr <SMTPConnection> SMTPTransport::getConnection()
{
	return m_connection;
}


void SMTPTransport::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	m_connection->disconnect();
	m_connection = null;
}


void SMTPTransport::noop()
{
	if (!isConnected())
		throw exceptions::not_connected();

	m_connection->sendRequest(SMTPCommand::NOOP());

	shared_ptr <SMTPResponse> resp = m_connection->readResponse();

	if (resp->getCode() != 250)
	{
		throw SMTPCommandError
			("NOOP", resp->getText(), resp->getCode(), resp->getEnhancedCode());
	}
}


void SMTPTransport::sendEnvelope
	(const mailbox& expeditor, const mailboxList& recipients,
	 const mailbox& sender, bool sendDATACommand,
	 const size_t size)
{
	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty())
		throw exceptions::no_recipient();
	else if (expeditor.isEmpty())
		throw exceptions::no_expeditor();


	const bool needReset = m_needReset;
	const bool hasPipelining = m_connection->hasExtension("PIPELINING") &&
		getInfos().getPropertyValue <bool>(getSession(),
			dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().PROPERTY_OPTIONS_PIPELINING);

	shared_ptr <SMTPResponse> resp;
	shared_ptr <SMTPCommandSet> commands = SMTPCommandSet::create(hasPipelining);

	// Emit a "RSET" command if we previously sent a message on this connection
	if (needReset)
		commands->addCommand(SMTPCommand::RSET());

	// Emit the "MAIL" command
	const bool hasSMTPUTF8 = m_connection->hasExtension("SMTPUTF8");
	const bool hasSize = m_connection->hasExtension("SIZE");

	if (!sender.isEmpty())
		commands->addCommand(SMTPCommand::MAIL(sender, hasSMTPUTF8, hasSize ? size : 0));
	else
		commands->addCommand(SMTPCommand::MAIL(expeditor, hasSMTPUTF8, hasSize ? size : 0));

	// Now, we will need to reset next time
	m_needReset = true;

	// Emit a "RCPT TO" command for each recipient
	for (size_t i = 0 ; i < recipients.getMailboxCount() ; ++i)
	{
		const mailbox& mbox = *recipients.getMailboxAt(i);
		commands->addCommand(SMTPCommand::RCPT(mbox, hasSMTPUTF8));
	}

	// Prepare sending of message data
	if (sendDATACommand)
		commands->addCommand(SMTPCommand::DATA());

	// Read response for "RSET" command
	if (needReset)
	{
		commands->writeToSocket(m_connection->getSocket(), m_connection->getTracer());

		resp = m_connection->readResponse();

		if (resp->getCode() != 250 &&
		    resp->getCode() != 200)   // RFC-876: << In reply to a RSET and/or a NOOP command,
		                              //             some servers reply "200" >>
		{
			disconnect();

			throw SMTPCommandError
				(commands->getLastCommandSent()->getText(), resp->getText(),
				 resp->getCode(), resp->getEnhancedCode());
		}
	}

	// Read response for "MAIL" command
	commands->writeToSocket(m_connection->getSocket(), m_connection->getTracer());

	if ((resp = m_connection->readResponse())->getCode() != 250)
	{
		// SIZE extension: insufficient system storage
		if (resp->getCode() == 452)
		{
			throw SMTPMessageSizeExceedsCurLimitsException
				(SMTPCommandError(commands->getLastCommandSent()->getText(), resp->getText(),
				 resp->getCode(), resp->getEnhancedCode()));
		}
		// SIZE extension: message size exceeds fixed maximum message size
		else if (resp->getCode() == 552)
		{
			throw SMTPMessageSizeExceedsMaxLimitsException
				(SMTPCommandError(commands->getLastCommandSent()->getText(), resp->getText(),
				 resp->getCode(), resp->getEnhancedCode()));
		}
		// Other error
		else
		{
			throw SMTPCommandError
				(commands->getLastCommandSent()->getText(), resp->getText(),
				 resp->getCode(), resp->getEnhancedCode());
		}
	}

	// Read responses for "RCPT TO" commands
	for (size_t i = 0 ; i < recipients.getMailboxCount() ; ++i)
	{
		commands->writeToSocket(m_connection->getSocket(), m_connection->getTracer());

		resp = m_connection->readResponse();

		if (resp->getCode() != 250 &&
		    resp->getCode() != 251)
		{
			// SIZE extension: insufficient system storage
			if (resp->getCode() == 452)
			{
				throw SMTPMessageSizeExceedsCurLimitsException
					(SMTPCommandError(commands->getLastCommandSent()->getText(), resp->getText(),
					 resp->getCode(), resp->getEnhancedCode()));
			}
			// SIZE extension: message size exceeds fixed maximum message size
			else if (resp->getCode() == 552)
			{
				throw SMTPMessageSizeExceedsMaxLimitsException
					(SMTPCommandError(commands->getLastCommandSent()->getText(), resp->getText(),
					 resp->getCode(), resp->getEnhancedCode()));
			}
			// Other error
			else
			{
				throw SMTPCommandError
					(commands->getLastCommandSent()->getText(), resp->getText(),
					 resp->getCode(), resp->getEnhancedCode());
			}
		}
	}

	// Read response for "DATA" command
	if (sendDATACommand)
	{
		commands->writeToSocket(m_connection->getSocket(), m_connection->getTracer());

		if ((resp = m_connection->readResponse())->getCode() != 354)
		{
			throw SMTPCommandError
				(commands->getLastCommandSent()->getText(), resp->getText(),
				 resp->getCode(), resp->getEnhancedCode());
		}
	}
}


void SMTPTransport::send
	(const mailbox& expeditor, const mailboxList& recipients,
	 utility::inputStream& is, const size_t size,
	 utility::progressListener* progress, const mailbox& sender)
{
	if (!isConnected())
		throw exceptions::not_connected();

	// Send message envelope
	sendEnvelope(expeditor, recipients, sender, /* sendDATACommand */ true, size);

	// Send the message data
	// Stream copy with "\n." to "\n.." transformation
	utility::outputStreamSocketAdapter sos(*m_connection->getSocket());
	utility::dotFilteredOutputStream fos(sos);

	utility::bufferedStreamCopy(is, fos, size, progress);

	fos.flush();

	// Send end-of-data delimiter
	m_connection->getSocket()->send("\r\n.\r\n");

	if (m_connection->getTracer())
	{
		m_connection->getTracer()->traceSendBytes(size);
		m_connection->getTracer()->traceSend(".");
	}

	shared_ptr <SMTPResponse> resp;

	if ((resp = m_connection->readResponse())->getCode() != 250)
	{
		throw SMTPCommandError
			("DATA", resp->getText(), resp->getCode(), resp->getEnhancedCode());
	}
}


void SMTPTransport::send
	(shared_ptr <vmime::message> msg, const mailbox& expeditor, const mailboxList& recipients,
	 utility::progressListener* progress, const mailbox& sender)
{
	if (!isConnected())
		throw exceptions::not_connected();

	// Generate the message with Internationalized Email support,
	// if this is supported by the SMTP server
	generationContext ctx(generationContext::getDefaultContext());
	ctx.setInternationalizedEmailSupport(m_connection->hasExtension("SMTPUTF8"));

	// If CHUNKING is not supported, generate the message to a temporary
	// buffer then use the send() method which takes an inputStream
	if (!m_connection->hasExtension("CHUNKING") ||
	    !getInfos().getPropertyValue <bool>(getSession(),
			dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().PROPERTY_OPTIONS_CHUNKING))

	{
		std::ostringstream oss;
		utility::outputStreamAdapter ossAdapter(oss);

		msg->generate(ctx, ossAdapter);

		const string& str(oss.str());

		utility::inputStreamStringAdapter isAdapter(str);

		send(expeditor, recipients, isAdapter, str.length(), progress, sender);
		return;
	}

	// Send message envelope
	const size_t msgSize = msg->getGeneratedSize(ctx);

	sendEnvelope(expeditor, recipients, sender,
		/* sendDATACommand */ false, msgSize);

	// Send the message by chunks
	SMTPChunkingOutputStreamAdapter chunkStream(m_connection, msgSize, progress);

	msg->generate(ctx, chunkStream);

	chunkStream.flush();
}



// Service infos

SMTPServiceInfos SMTPTransport::sm_infos(false);


const serviceInfos& SMTPTransport::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& SMTPTransport::getInfos() const
{
	return sm_infos;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

