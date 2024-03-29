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

#include "vmime/parsingContext.hpp"


namespace vmime {


parsingContext::parsingContext()
	: m_headerParseErrorRecovery(vmime::headerParseRecoveryMethod::SKIP_LINE) {

}


parsingContext::parsingContext(const parsingContext& ctx)
	: context(ctx),
	  m_headerParseErrorRecovery(vmime::headerParseRecoveryMethod::SKIP_LINE) {

}


parsingContext& parsingContext::getDefaultContext() {

	static thread_local parsingContext ctx;
	return ctx;
}


headerParseRecoveryMethod::headerLineError parsingContext::getHeaderParseErrorRecoveryMethod() const {

	return m_headerParseErrorRecovery;
}


void parsingContext::setHeaderParseErrorRecoveryMethod(
	const headerParseRecoveryMethod::headerLineError recoveryMethod
) {

	m_headerParseErrorRecovery = recoveryMethod;
}


bool parsingContext::getHeaderRecoveryNeeded() const {

	return m_headerParseRecoveryNeeded;
}


void parsingContext::setHeaderRecoveryNeeded(bool needed) {

	m_headerParseRecoveryNeeded = needed;
}


bool parsingContext::getUseMyHostname() const {

	return m_useMyHostname;
}


void parsingContext::setUseMyHostname(bool useMyHostname) {

	m_useMyHostname = useMyHostname;
}


} // vmime
