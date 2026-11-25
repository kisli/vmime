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

#include "vmime/exception.hpp"
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


// Write the address of a mailbox in a MAIL or RCPT command, as in header
// fields. With SMTPUTF8, it is written in UTF-8, with the domain part as
// U-labels (RFC 6531, section 3.3). Without it, the domain part must be
// written as IDNA A-labels, even if Internationalized Email is enabled in
// the default context.
static void writeAddress(std::ostream& cmd, const mailbox& mbox, const bool utf8) {

	generationContext ctx(generationContext::getDefaultContext());
	ctx.setInternationalizedEmailSupport(utf8);
	ctx.setMaxLineLength(lineLengthLimits::infinite);

	vmime::utility::outputStreamAdapter cmd2(cmd);
	mbox.getEmail().generate(ctx, cmd2);
}


// Write a value encoded as xtext (RFC 3461, section 4)
static void writeXText(std::ostream& cmd, const string& value) {

	for (const char c : value) {

		const unsigned char k = static_cast <unsigned char>(c);

		if (k >= '!' && k <= '~' && c != '+' && c != '=') {
			cmd << c;
		} else {
			cmd << '+' << "0123456789ABCDEF"[k / 16] << "0123456789ABCDEF"[k % 16];
		}
	}
}


// Write the ORCPT parameter of a RCPT command: when a message is initially
// submitted, this is the same address as in the command (RFC 3461, section 4.2)
static void writeOriginalRecipient(std::ostream& cmd, const string& address) {

	bool ascii = true;

	for (const char c : address) {

		if (static_cast <unsigned char>(c) >= 128) {
			ascii = false;
		}
	}

	utility::details::classicOStringStream param;
	param << dsn::ORCPT << "=";

	if (ascii) {

		param << "rfc822;";
		writeXText(param, address);

	} else {

		// The "rfc822" type only allows US-ASCII addresses: use the "utf-8" type,
		// in the utf-8-addr-unitext form (RFC 6533, section 3)
		param << "utf-8;";

		for (const char c : address) {

			const unsigned char k = static_cast <unsigned char>(c);

			if (k <= ' ' || k == 127 || c == '\\' || c == '+' || c == '=') {
				param << "\\x{" << "0123456789ABCDEF"[k / 16] << "0123456789ABCDEF"[k % 16] << "}";
			} else {
				param << c;
			}
		}
	}

	// Servers must accept an ORCPT parameter of up to 500 characters (RFC 3461,
	// sections 4.2 and 5.4): as it is optional, it is not sent if it is longer
	const string str = param.str();

	if (str.length() <= 500) {
		cmd << " " << str;
	}
}


// static
shared_ptr <SMTPCommand> SMTPCommand::MAIL(
	const mailbox& mbox, const bool utf8,
	const shared_ptr <const DSNAttributes>& dsnAttrs
) {

	return MAIL(mbox, utf8, BODY_7BIT, 0, dsnAttrs);
}


// static
shared_ptr <SMTPCommand> SMTPCommand::MAIL(
	const mailbox& mbox, const bool utf8, const BodyType bodyType, const size_t size,
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
		// The envelope identifier is sent as is, encoded as xtext (RFC 3461, section 4.4)
		cmd << " " << dsn::ENVID << "=";
		writeXText(cmd, dsnAttrs->getEnvelopId());
	}

	if (utf8) {
		cmd << " SMTPUTF8";
	}

	// Binary data must be declared (RFC 3030, section 3), as well as 8-bit
	// data (RFC 6152, section 3), and an internationalized message contains
	// 8-bit data (RFC 6531, section 1)
	if (bodyType == BODY_BINARYMIME) {
		cmd << " BODY=BINARYMIME";
	} else if (utf8 || bodyType == BODY_8BITMIME) {
		cmd << " BODY=8BITMIME";
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

	utility::details::classicOStringStream address;
	writeAddress(address, mbox, utf8);

	utility::details::classicOStringStream cmd;
	cmd << "RCPT TO:<" << address.str() << ">";

	if (dsnAttrs && !dsnAttrs->getNotificationConditions().empty()) {
		cmd << " " << dsn::NOTIFY << "=" << dsnAttrs->getNotificationConditions();
	}

	if (dsnAttrs) {
		writeOriginalRecipient(cmd, address.str());
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

	// CR and LF may only end a command (RFC 5321, section 2.3.8): elsewhere,
	// eg. in an address, they would start another one, as NUL might
	if (text.find_first_of(string("\r\n\0", 3)) != string::npos) {
		throw exceptions::invalid_argument();
	}

	if (traceText.empty()) {
		return shared_ptr <SMTPCommand>(new SMTPCommand(text, text));
	} else {
		return shared_ptr <SMTPCommand>(new SMTPCommand(text, traceText));
	}
}


string SMTPCommand::getText() const {

	return m_text;
}


string SMTPCommand::getTraceText() const {

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
