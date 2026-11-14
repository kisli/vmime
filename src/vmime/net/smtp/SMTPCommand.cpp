//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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
#include "vmime/net/tracer.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/details/classicOStringStream.hpp"


namespace vmime {
namespace net {
namespace smtp {


SMTPCommand::SMTPCommand(const string& text, const string& traceText)
	: m_text(text),
	  m_traceText(traceText)
{
}


// static
shared_ptr <SMTPCommand> SMTPCommand::EHLO(const string& hostname) {

	utility::details::classicOStringStream cmd;
	cmd << "EHLO " << hostname;

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::HELO(const string& hostname) {

	utility::details::classicOStringStream cmd;
	cmd << "HELO " << hostname;

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::AUTH(const string& mechName) {

	utility::details::classicOStringStream cmd;
	cmd << "AUTH " << mechName;

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::AUTH(const string& mechName, const std::string& initialResponse) {

	utility::details::classicOStringStream cmd;
	cmd << "AUTH " << mechName << " " << initialResponse;

	utility::details::classicOStringStream trace;
	trace << "AUTH " << mechName << " {initial-response}";

	return createCommand(cmd.str(), trace.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::STARTTLS() {

	return createCommand("STARTTLS");
}


// Write the address of a mailbox in a MAIL or RCPT command
static void writeAddress(std::ostream& cmd, const mailbox& mbox, const bool utf8) {

	if (utf8) {

		cmd << mbox.getEmail().toText().getConvertedText(vmime::charsets::UTF_8);

	} else {

		// Without SMTPUTF8, the domain part must be written as IDNA A-labels,
		// even if Internationalized Email is enabled in the default context
		generationContext ctx(generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(false);

		vmime::utility::outputStreamAdapter cmd2(cmd);
		mbox.getEmail().generate(ctx, cmd2);
	}
}


// static
shared_ptr <SMTPCommand> SMTPCommand::MAIL(
	const mailbox& mbox, const bool utf8,
	const shared_ptr <const DSNAttributes>& dsnAttrs
) {

	return MAIL(mbox, utf8, 0, dsnAttrs);
}


// static
shared_ptr <SMTPCommand> SMTPCommand::MAIL(
	const mailbox& mbox, const bool utf8, const size_t size,
	const shared_ptr <const DSNAttributes>& dsnAttrs
) {

	utility::details::classicOStringStream cmd;
	cmd << "MAIL FROM:<";
	writeAddress(cmd, mbox, utf8);
	cmd << ">";

	if (dsnAttrs && !dsnAttrs->getReturnFormat().empty()) {
		cmd << " " << dsn::RET << "=" << dsnAttrs->getReturnFormat();
	}
	if (dsnAttrs && !dsnAttrs->getEnvelopId().empty()) {
		cmd << " " << dsn::ENVID << "=<" << dsnAttrs->getEnvelopId() << ">";
	}

	if (utf8) {
		cmd << " SMTPUTF8";
	}

	if (size != 0) {
		cmd << " SIZE=" << size;
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::RCPT(
	const mailbox& mbox, const bool utf8,
	const shared_ptr <const DSNAttributes>& dsnAttrs
) {

	utility::details::classicOStringStream cmd;
	cmd << "RCPT TO:<";
	writeAddress(cmd, mbox, utf8);
	cmd << ">";

	if (dsnAttrs && !dsnAttrs->getNotificationConditions().empty()) {
		cmd << " " << dsn::NOTIFY << "=" << dsnAttrs->getNotificationConditions();
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::RSET() {

	return createCommand("RSET");
}


// static
shared_ptr <SMTPCommand> SMTPCommand::DATA() {

	return createCommand("DATA");
}


// static
shared_ptr <SMTPCommand> SMTPCommand::BDAT(const size_t chunkSize, const bool last) {

	utility::details::classicOStringStream cmd;
	cmd << "BDAT " << chunkSize;

	if (last) {
		cmd << " LAST";
	}

	return createCommand(cmd.str());
}


// static
shared_ptr <SMTPCommand> SMTPCommand::NOOP() {

	return createCommand("NOOP");
}


// static
shared_ptr <SMTPCommand> SMTPCommand::QUIT() {

	return createCommand("QUIT");
}


// static
shared_ptr <SMTPCommand> SMTPCommand::createCommand(const string& text, const string& traceText) {

	if (traceText.empty()) {
		return shared_ptr <SMTPCommand>(new SMTPCommand(text, text));
	} else {
		return shared_ptr <SMTPCommand>(new SMTPCommand(text, traceText));
	}
}


const string SMTPCommand::getText() const {

	return m_text;
}


const string SMTPCommand::getTraceText() const {

	return m_traceText;
}


void SMTPCommand::writeToSocket(const shared_ptr <socket>& sok, const shared_ptr <tracer>& tr) {

	sok->send(m_text + "\r\n");

	if (tr) {
		tr->traceSend(m_traceText);
	}
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
