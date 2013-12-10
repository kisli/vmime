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

#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace utility {


inputStreamStringAdapter::inputStreamStringAdapter(const string& buffer)
	: m_buffer(buffer), m_begin(0), m_end(buffer.length()), m_pos(0)
{
}


inputStreamStringAdapter::inputStreamStringAdapter(const string& buffer,
	const size_t begin, const size_t end)
	: m_buffer(buffer), m_begin(begin), m_end(end), m_pos(begin)
{
}


bool inputStreamStringAdapter::eof() const
{
	return (m_pos >= m_end);
}


void inputStreamStringAdapter::reset()
{
	m_pos = m_begin;
}


size_t inputStreamStringAdapter::read
	(byte_t* const data, const size_t count)
{
	if (m_pos + count >= m_end)
	{
		const size_t remaining = m_end - m_pos;

		std::copy(m_buffer.begin() + m_pos, m_buffer.end(), data);
		m_pos = m_end;
		return (remaining);
	}
	else
	{
		std::copy(m_buffer.begin() + m_pos, m_buffer.begin() + m_pos + count, data);
		m_pos += count;
		return (count);
	}
}


size_t inputStreamStringAdapter::skip(const size_t count)
{
	if (m_pos + count >= m_end)
	{
		const size_t remaining = m_end - m_pos;
		m_pos = m_end;
		return (remaining);
	}
	else
	{
		m_pos += count;
		return (count);
	}
}


size_t inputStreamStringAdapter::getPosition() const
{
	return m_pos - m_begin;
}


void inputStreamStringAdapter::seek(const size_t pos)
{
	if (m_begin + pos <= m_end)
		m_pos = m_begin + pos;
}


} // utility
} // vmime

