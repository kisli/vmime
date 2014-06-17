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
	size_t getMaxLineLength() const;

	/** Sets the maximum line length used when generating messages.
	  * You may use the constants lineLengthLimits::convenient,
	  * lineLengthLimits::max and lineLengthLimits::infinite.
	  *
	  * @param maxLineLength new maximum line length, in bytes
	  */
	void setMaxLineLength(const size_t maxLineLength);

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

	/** Modes available for generating values in parameterized header fields.
	  */
	enum EncodedParameterValueModes
	{
		PARAMETER_VALUE_NO_ENCODING,          /**< Only generate 7-bit (ASCII-only) values,
		                                           even if the value contains non-ASCII chars or
		                                           if folding is needed. */
		PARAMETER_VALUE_RFC2047_ONLY,         /**< Only generate RFC-2047 values (do not use
		                                           RFC-2231). This is non-standard but most
		                                           mail clients support it. */
		PARAMETER_VALUE_RFC2231_ONLY,         /**< Only generate RFC-2231 values (do not use
		                                           RFC-2047). Some mail clients may not support
		                                           it. This is the default. */
		PARAMETER_VALUE_RFC2231_AND_RFC2047   /**< Generate both RFC-2047- and RFC-2231-encoded
		                                           values. */
	};

	/** Sets the mode used for generating parameter values in a parameterized
	  * header field (see parameterizedHeaderField class).
	  *
	  * PARAMETER_VALUE_NO_ENCODING or PARAMETER_VALUE_RFC2047_ONLY
	  * can be used for compatibility with implementations that do not
	  * understand RFC-2231, to generate a normal parameter value.
	  * PARAMETER_VALUE_RFC2047_ONLY is non-standard (and expressly
	  * prohibited by the RFC) but most mail clients support it.
	  *
	  * Notice: if both the normal value and the extended value are present,
	  * the latter can be ignored by mail processing systems. This may lead
	  * to annoying problems, for example, with strange names of attachments
	  * with all but 7-bit ascii characters removed, etc. Either
	  * PARAMETER_VALUE_RFC2231_ONLY or PARAMETER_VALUE_RFC2047_ONLY should
	  * be preferred over PARAMETER_VALUE_RFC2231_AND_RFC2047, not to create
	  * a normal value if the extended value is to be generated.
	  *
	  * @param mode parameter value generation mode
	  */
	void setEncodedParameterValueMode(const EncodedParameterValueModes mode);

	/** Returns the mode used for generating parameter values in a parameterized
	  * header field (see parameterizedHeaderField class).
	  *
	  * @return parameter value generation mode
	  */
	EncodedParameterValueModes getEncodedParameterValueMode() const;

	/** Returns the default context used for generating messages.
	  *
	  * @return a reference to the default generation context
	  */
	static generationContext& getDefaultContext();

	generationContext& operator=(const generationContext& ctx);
	void copyFrom(const generationContext& ctx);

protected:

	size_t m_maxLineLength;

	string m_prologText;
	string m_epilogText;

	EncodedParameterValueModes m_paramValueMode;
};


} // vmime


#endif // VMIME_GENERATIONCONTEXT_HPP_INCLUDED
