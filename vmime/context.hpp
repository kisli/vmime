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

#ifndef VMIME_CONTEXT_HPP_INCLUDED
#define VMIME_CONTEXT_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/charsetConverterOptions.hpp"


namespace vmime
{


/** Holds configuration parameters used either for parsing or generating messages.
  */

class VMIME_EXPORT context : public object
{
public:

	virtual ~context();

	/** Returns whether support for Internationalized Email Headers (RFC-6532)
	  * is enabled.
	  *
	  * @return true if RFC-6532 support is enabled, false otherwise
	  */
	bool getInternationalizedEmailSupport() const;

	/** Enables or disables support for Internationalized Email Headers (RFC-6532).
	  * This is disabled by default, and should be used only with servers
	  * which support it (eg. SMTP servers with SMTPUTF8 extension).
	  *
	  * @param support true if RFC-6532 support is enabled, false otherwise
	  */
	void setInternationalizedEmailSupport(const bool support);

	/** Returns options used currently for charset conversions by the parser and/or
	  * the generator. See charsetConverterOptions class for more information.
	  *
	  * @return current charset conversion options
	  */
	const charsetConverterOptions& getCharsetConversionOptions() const;

	/** Sets the options used currently for charset conversions by the parser and/or
	  * the generator. See charsetConverterOptions class for more information.
	  *
	  * @param opts new charset conversion options
	  */
	void setCharsetConversionOptions(const charsetConverterOptions& opts);

	/** Switches between contexts temporarily.
	  */
	template <typename CTX_CLASS>
	class switcher
	{
	public:

		/** Switches to the specified context.
		  * Default context will temporarily use the data of the specified
		  * new context during the lifetime of this object.
		  *
		  * @param newCtx new context
		  */
		switcher(CTX_CLASS& newCtx)
			: m_oldCtxData(CTX_CLASS::getDefaultContext()), m_newCtx(&newCtx)
		{
			CTX_CLASS::getDefaultContext().copyFrom(newCtx);
		}

		/** Restores back saved context.
		  */
		~switcher()
		{
			CTX_CLASS::getDefaultContext().copyFrom(m_oldCtxData);
		}

	private:

		CTX_CLASS  m_oldCtxData;
		CTX_CLASS* m_newCtx;
	};

protected:

	context();
	context(const context& ctx);

	virtual context& operator=(const context& ctx);
	void copyFrom(const context& ctx);

	bool m_internationalizedEmail;
	charsetConverterOptions m_charsetConvOptions;
};


} // vmime


#endif // VMIME_CONTEXT_HPP_INCLUDED
