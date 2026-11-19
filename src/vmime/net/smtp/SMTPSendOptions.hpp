//
// VMime library (http://www.vmime.org)
// Copyright (C) 2020 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_SMTP_SMTPSENDOPTIONS_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPSENDOPTIONS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/transport.hpp"

#include "vmime/net/smtp/DSNAttributes.hpp"


namespace vmime {
namespace net {
namespace smtp {


/** Holds options for sending messages over SMTP.
  */
class VMIME_EXPORT SMTPSendOptions : public transport::sendOptions {

public:

	/** Constructs an empty SMTPSendOptions object.
	  */
	SMTPSendOptions() = default;

	/** Constructs a new SMTPSendOptions object by copying an existing object.
	  *
	  * @param dsn object to copy
	  */
	SMTPSendOptions(const SMTPSendOptions& dsn) = default;

	/** Constructs a new SMTPSendOptions object by moving an existing object.
	  *
	  * @param dsn object (Rvalue reference) to move from.
	  */
	SMTPSendOptions(SMTPSendOptions&& dsn) = default;

	~SMTPSendOptions() = default;

	/** Set DSN attributes to use when sending a message.
	  *
	  * @param dsnAttribs DSN attributes
	  */
	void setDSNAttributes(const shared_ptr <DSNAttributes>& dsnAttribs);

	/** Return DSN attributes used when sending a message (const version).
	  *
	  * @return DSN attributes, if set
	  */
	shared_ptr <const DSNAttributes> getDSNAttributes() const;

	/** Return DSN attributes used when sending a message.
	  *
	  * @return DSN attributes, if set
	  */
	shared_ptr <DSNAttributes> getDSNAttributes();

	/** Set whether the message is an internationalized message, ie. whether
	  * its header fields contain UTF-8 (RFC 6532). If so, the message is sent
	  * with the SMTPUTF8 extension (RFC 6531), which must be supported by the
	  * server.
	  *
	  * This is needed when sending raw message data with UTF-8 header fields:
	  * VMime does not parse the data, and only checks the envelope addresses.
	  * When sending a message object, VMime detects it by itself.
	  *
	  * @param internationalized true if the message is an internationalized
	  * message, false otherwise (default)
	  */
	void setInternationalizedMessage(const bool internationalized);

	/** Return whether the message is an internationalized message
	  * (RFC 6532).
	  *
	  * @return true if the message is an internationalized message,
	  * false otherwise
	  */
	bool isInternationalizedMessage() const;

	/** Set whether the message contains 8-bit data. If so, the message is
	  * sent with the BODY=8BITMIME parameter (RFC 6152), and the 8BITMIME
	  * extension must be supported by the server.
	  *
	  * VMime detects by itself the parts declared as 8-bit in a message
	  * object. Otherwise, the data is checked while it is sent, and an
	  * SMTPUndeclared8bitDataException is thrown if it contains 8-bit
	  * data: the message is not sent, and you have to send it again with
	  * this option. This is the case for raw message data which contains
	  * 8-bit data.
	  *
	  * @param eightBit true if the message contains 8-bit data, false
	  * otherwise (default)
	  */
	void set8bitMessage(const bool eightBit);

	/** Return whether the message contains 8-bit data.
	  *
	  * @return true if the message contains 8-bit data, false otherwise
	  */
	bool is8bitMessage() const;

	/** Set whether the message contains binary data, ie. NUL, or lines
	  * longer than 998 octets (RFC 2045, section 2.9). If so, the message is
	  * sent with the BODY=BINARYMIME parameter (RFC 3030), and the BINARYMIME
	  * and CHUNKING extensions must be supported by the server. The message
	  * data is then sent as is, without converting its line endings.
	  *
	  * VMime detects by itself the parts declared as binary in a message
	  * object. Otherwise, the data is checked while it is sent, and an
	  * SMTPUndeclaredBinaryDataException is thrown if it contains binary
	  * data: the message is not sent, and you have to send it again with
	  * this option. This is the case for raw message data which contains
	  * binary data.
	  *
	  * @param binary true if the message contains binary data, false
	  * otherwise (default)
	  */
	void setBinaryMessage(const bool binary);

	/** Return whether the message contains binary data.
	  *
	  * @return true if the message contains binary data, false otherwise
	  */
	bool isBinaryMessage() const;

	/** Set whether the parts of a message object which cannot be sent as
	  * is may be converted, so that the message can be sent anyway (the
	  * message object itself is not modified):
	  *
	  * - the parts declared as binary, if the server does not support the
	  *   BINARYMIME extension (RFC 3030, section 3), are encoded in Base64,
	  *   which keeps their data as is;
	  * - the parts declared as 7bit or 8bit which contain binary data (NUL,
	  *   or lines longer than 998 octets), which is not valid (RFC 2045,
	  *   sections 2.7 and 2.8), are encoded in Quoted-Printable (text) or
	  *   Base64;
	  * - the parts which contain 8-bit data, if the server does not support
	  *   the 8BITMIME extension (RFC 6152, section 3), are encoded in
	  *   Quoted-Printable (text) or Base64.
	  *
	  * If the conversion is disabled, an exception is thrown instead. Raw
	  * message data is never converted.
	  *
	  * @param convert true if parts may be converted (default), false
	  * otherwise
	  */
	void setConvertParts(const bool convert);

	/** Return whether the parts of a message object which cannot be sent
	  * as is may be converted.
	  *
	  * @return true if parts may be converted, false otherwise
	  */
	bool getConvertParts() const;

private:

	shared_ptr <DSNAttributes> m_dsnAttribs;
	bool m_internationalizedMessage = false;
	bool m_8bitMessage = false;
	bool m_binaryMessage = false;
	bool m_convertParts = true;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_NET_SMTP_SMTPSENDOPTIONS_HPP_INCLUDED
