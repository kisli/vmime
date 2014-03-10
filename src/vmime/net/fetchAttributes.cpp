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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/fetchAttributes.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <algorithm>


namespace vmime {
namespace net {


fetchAttributes::fetchAttributes()
	: m_predefinedAttribs(0)
{
}


fetchAttributes::fetchAttributes(const int attribs)
	: m_predefinedAttribs(attribs)
{
}


fetchAttributes::fetchAttributes(const fetchAttributes& attribs)
	: object()
{
	m_predefinedAttribs = attribs.m_predefinedAttribs;
	m_headers = attribs.m_headers;
}


void fetchAttributes::add(const int attribs)
{
	m_predefinedAttribs |= attribs;
}


void fetchAttributes::add(const string& header)
{
	m_headers.push_back(utility::stringUtils::toLower(header));
}


bool fetchAttributes::has(const int attribs) const
{
	return (m_predefinedAttribs & attribs) != 0;
}


bool fetchAttributes::has(const string& header) const
{
	return std::find(m_headers.begin(), m_headers.end(), utility::stringUtils::toLower(header)) != m_headers.end();
}


const std::vector <string> fetchAttributes::getHeaderFields() const
{
	return m_headers;
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
