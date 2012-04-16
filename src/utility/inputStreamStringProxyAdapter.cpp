//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2012 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/inputStreamStringProxyAdapter.hpp"
#include "vmime/utility/stringProxy.hpp"


namespace vmime {
namespace utility {


inputStreamStringProxyAdapter::inputStreamStringProxyAdapter(const stringProxy& buffer)
	: m_buffer(buffer), m_pos(0)
{
}


bool inputStreamStringProxyAdapter::eof() const
{
	return (m_pos >= m_buffer.length());
}


void inputStreamStringProxyAdapter::reset()
{
	m_pos = 0;
}


stream::size_type inputStreamStringProxyAdapter::read
	(value_type* const data, const size_type count)
{
	const size_type remaining = m_buffer.length() - m_pos;

	if (count > remaining)
	{
		std::copy(m_buffer.it_begin() + m_pos, m_buffer.it_end(), data);
		m_pos = m_buffer.length();
		return (remaining);
	}
	else
	{
		std::copy(m_buffer.it_begin() + m_pos, m_buffer.it_begin() + m_pos + count, data);
		m_pos += count;
		return (count);
	}
}


stream::size_type inputStreamStringProxyAdapter::skip(const size_type count)
{
	const size_type remaining = m_buffer.length() - m_pos;

	if (count > remaining)
	{
		m_pos = m_buffer.length();
		return (remaining);
	}
	else
	{
		m_pos += count;
		return (count);
	}
}


stream::size_type inputStreamStringProxyAdapter::getPosition() const
{
	return m_pos;
}


void inputStreamStringProxyAdapter::seek(const size_type pos)
{
	if (pos <= m_buffer.length())
		m_pos = pos;
}


} // utility
} // vmime

