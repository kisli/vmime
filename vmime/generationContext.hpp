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

#ifndef VMIME_GENERATIONCONTEXT_HPP_INCLUDED
#define VMIME_GENERATIONCONTEXT_HPP_INCLUDED


#include "vmime/context.hpp"


namespace vmime
{


/** Holds configuration parameters used for generating messages.
  */

class VMIME_EXPORT generationContext : public context
{
public:

	generationContext();
	generationContext(const generationContext& ctx);

	/** Returns the current maximum line length used when generating messages.
	  *
	  * @return current maximum line length, in bytes
	  */
	string::size_type getMaxLineLength() const;

	/** Sets the maximum line length used when generating messages.
	  * You may use the constants lineLengthLimits::convenient,
	  * lineLengthLimits::max and lineLengthLimits::infinite.
	  *
	  * @param maxLineLength new maximum line length, in bytes
	  */
	void setMaxLineLength(const string::size_type maxLineLength);

	/** Returns the current prolog text used when generating MIME body parts.
	  *
	  * @return current MIME prolog text
	  */
	const string getPrologText() const;

	/** Sets the prolog text used when generating MIME body parts. This text
	  * appears before the part, and should be displayed by MUAs which do not
	  * support MIME. This should be 7-bit ASCII text only.
	  *
	  * @param prologText MIME prolog text
	  */
	void setPrologText(const string& prologText);

	/** Returns the current epilog text used when generating MIME body parts.
	  *
	  * @return current MIME epilog text
	  */
	const string getEpilogText() const;

	/** Sets the epilog text used when generating MIME body parts. This test
	  * appears after the part, and should be displayed by MUAs which do not
	  * support MIME. This should be 7-bit ASCII text only.
	  */
	void setEpilogText(const string& epilogText);

	/** Returns the default context used for generating messages.
	  *
	  * @return a reference to the default generation context
	  */
	static generationContext& getDefaultContext();

	generationContext& operator=(const generationContext& ctx);
	void copyFrom(const generationContext& ctx);

protected:

	string::size_type m_maxLineLength;

	string m_prologText;
	string m_epilogText;
};


} // vmime


#endif // VMIME_GENERATIONCONTEXT_HPP_INCLUDED
