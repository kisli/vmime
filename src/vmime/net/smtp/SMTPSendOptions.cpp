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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/smtp/SMTPSendOptions.hpp"


namespace vmime {
namespace net {
namespace smtp {


void SMTPSendOptions::setDSNAttributes(const shared_ptr <DSNAttributes>& dsnAttribs) {

	m_dsnAttribs = dsnAttribs;
}


shared_ptr <DSNAttributes> SMTPSendOptions::getDSNAttributes() {

	return m_dsnAttribs;
}


shared_ptr <const DSNAttributes> SMTPSendOptions::getDSNAttributes() const {

	return m_dsnAttribs;
}


void SMTPSendOptions::setInternationalizedMessage(const bool internationalized) {

	m_internationalizedMessage = internationalized;
}


bool SMTPSendOptions::isInternationalizedMessage() const {

	return m_internationalizedMessage;
}


void SMTPSendOptions::set8bitMessage(const bool eightBit) {

	m_8bitMessage = eightBit;
}


bool SMTPSendOptions::is8bitMessage() const {

	return m_8bitMessage;
}


void SMTPSendOptions::setBinaryMessage(const bool binary) {

	m_binaryMessage = binary;
}


bool SMTPSendOptions::isBinaryMessage() const {

	return m_binaryMessage;
}


void SMTPSendOptions::setConvertParts(const bool convert) {

	m_convertParts = convert;
}


bool SMTPSendOptions::getConvertParts() const {

	return m_convertParts;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
