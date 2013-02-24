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

#include "vmime/context.hpp"


namespace vmime
{


context::context()
	: m_internationalizedEmail(false)
{
}


context::context(const context& ctx)
	: object(),
	  m_internationalizedEmail(ctx.m_internationalizedEmail)
{
}


context::~context()
{
}


bool context::getInternationalizedEmailSupport() const
{
	return m_internationalizedEmail;
}


void context::setInternationalizedEmailSupport(const bool support)
{
	m_internationalizedEmail = support;
}


const charsetConverterOptions& context::getCharsetConversionOptions() const
{
	return m_charsetConvOptions;
}


void context::setCharsetConversionOptions(const charsetConverterOptions& opts)
{
	m_charsetConvOptions = opts;
}


context& context::operator=(const context& ctx)
{
	copyFrom(ctx);
	return *this;
}


void context::copyFrom(const context& ctx)
{
	m_internationalizedEmail = ctx.m_internationalizedEmail;
	m_charsetConvOptions = ctx.m_charsetConvOptions;
}


} // vmime
