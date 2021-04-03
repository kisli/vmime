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
	const shared_ptr <const DSNAttributes> getDSNAttributes() const;

	/** Return DSN attributes used when sending a message.
	  *
	  * @return DSN attributes, if set
	  */
	const shared_ptr <DSNAttributes> getDSNAttributes();

private:

	shared_ptr <DSNAttributes> m_dsnAttribs;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_NET_SMTP_SMTPSENDOPTIONS_HPP_INCLUDED
