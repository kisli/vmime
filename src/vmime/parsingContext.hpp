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

#ifndef VMIME_PARSINGCONTEXT_HPP_INCLUDED
#define VMIME_PARSINGCONTEXT_HPP_INCLUDED


#include "vmime/context.hpp"


namespace vmime {

/** Provides runtime configurable options to provide flexibility in header parsing
  */
struct headerParseRecoveryMethod {

	enum headerLineError {
		SKIP_LINE = 0,
		/* APPEND_TO_PREVIOUS_LINE = 1, */
		ASSUME_END_OF_HEADERS = 2
	};
};

/** Holds configuration parameters used for parsing messages.
  */
class VMIME_EXPORT parsingContext : public context {

	friend class headerField;

public:

	parsingContext();
	parsingContext(const parsingContext& ctx);

	/** Returns the default context used for parsing messages.
	  *
	  * @return a reference to the default parsing context
	  */
	static parsingContext& getDefaultContext();

	/** Sets the recovery method when parsing a header encounters an error
	  * such as a failed fold or missing new line.
	  *
	  * @param recoveryMethod is one of vmime::headerParseRecoveryMethod.
	  * Defaults to vmime::headerParseRecoveryMethod::SKIP_LINE.
	  */
	void setHeaderParseErrorRecoveryMethod(const headerParseRecoveryMethod::headerLineError recoveryMethod);

	/** Return the recovery method when parsing a header encounters an error.
	  *
	  * @return is an enum from vmime::headerParseRecoveryMethod
	  */
	headerParseRecoveryMethod::headerLineError getHeaderParseErrorRecoveryMethod() const;

	/** Returns a boolean indicating if utilizing the header recovery mechanism
	  *  was necessary.
	  *
	  * @retval true The header recovery mechanism was necessary when parsing
	  * @retval false The header recovery mechanism was not necessary when parsing
	  */
	bool getHeaderRecoveryNeeded() const;

protected:

	headerParseRecoveryMethod::headerLineError m_headerParseErrorRecovery;

	/** Flag to indicate if the header recovery mechanism was used while parsing
	  *  as only one method is ever in use, a simple boolean is sufficent
	  */
	bool m_recovery_needed{false};

	/** Sets a flag indicating that the header recovery mechanism was required
	  *
	  * This should only be called from headerField::parseNext
	  */
	void setHeaderRecoveryNeeded(bool needed);
};


} // vmime


#endif // VMIME_PARSINGCONTEXT_HPP_INCLUDED
