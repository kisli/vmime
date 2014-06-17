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

#include "vmime/generationContext.hpp"


namespace vmime
{


generationContext::generationContext()
	: m_maxLineLength(lineLengthLimits::convenient),
	  m_prologText("This is a multi-part message in MIME format. Your mail reader " \
	               "does not understand MIME message format."),
	  m_epilogText(""),
	  m_paramValueMode(PARAMETER_VALUE_RFC2231_ONLY)
{
}


generationContext::generationContext(const generationContext& ctx)
	: context(ctx),
	  m_maxLineLength(ctx.m_maxLineLength),
	  m_prologText(ctx.m_prologText),
	  m_epilogText(ctx.m_epilogText)
{
}


generationContext& generationContext::getDefaultContext()
{
	static generationContext ctx;
	return ctx;
}


size_t generationContext::getMaxLineLength() const
{
	return m_maxLineLength;
}


void generationContext::setMaxLineLength(const size_t maxLineLength)
{
	m_maxLineLength = maxLineLength;
}


const string generationContext::getPrologText() const
{
	return m_prologText;
}


void generationContext::setPrologText(const string& prologText)
{
	m_prologText = prologText;
}


const string generationContext::getEpilogText() const
{
	return m_epilogText;
}


void generationContext::setEpilogText(const string& epilogText)
{
	m_epilogText = epilogText;
}


void generationContext::setEncodedParameterValueMode(const EncodedParameterValueModes mode)
{
	m_paramValueMode = mode;
}


generationContext::EncodedParameterValueModes
	generationContext::getEncodedParameterValueMode() const
{
	return m_paramValueMode;
}


generationContext& generationContext::operator=(const generationContext& ctx)
{
	copyFrom(ctx);
	return *this;
}


void generationContext::copyFrom(const generationContext& ctx)
{
	context::copyFrom(ctx);

	m_maxLineLength = ctx.m_maxLineLength;
	m_prologText = ctx.m_prologText;
	m_epilogText = ctx.m_epilogText;
	m_paramValueMode = ctx.m_paramValueMode;
}


} // vmime
