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

#include "vmime/utility/seekableInputStreamRegionAdapter.hpp"


namespace vmime {
namespace utility {


seekableInputStreamRegionAdapter::seekableInputStreamRegionAdapter
	(ref <seekableInputStream> stream, const size_type begin, const size_type length)
	: m_stream(stream), m_begin(begin), m_length(length)
{
}


bool seekableInputStreamRegionAdapter::eof() const
{
	return getPosition() >= m_length;
}


void seekableInputStreamRegionAdapter::reset()
{
	m_stream->seek(m_begin);
}


stream::size_type seekableInputStreamRegionAdapter::read
	(value_type* const data, const size_type count)
{
	if (getPosition() + count >= m_length)
	{
		const size_type remaining = m_length - getPosition();
		return m_stream->read(data, remaining);
	}
	else
	{
		return m_stream->read(data, count);
	}
}


stream::size_type seekableInputStreamRegionAdapter::skip(const size_type count)
{
	if (getPosition() + count >= m_length)
	{
		const size_type remaining = m_length - getPosition();
		m_stream->skip(remaining);
		return remaining;
	}
	else
	{
		m_stream->skip(count);
		return count;
	}
}


stream::size_type seekableInputStreamRegionAdapter::getPosition() const
{
	return m_stream->getPosition() - m_begin;
}


void seekableInputStreamRegionAdapter::seek(const size_type pos)
{
	m_stream->seek(m_begin + pos);
}


} // utility
} // vmime

