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

#include "vmime/utility/stringProxy.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"

#include <iterator>
#include <algorithm>


namespace vmime {
namespace utility {


stringProxy::stringProxy()
	: m_start(0), m_end(0)
{
}


stringProxy::stringProxy(const stringProxy& s)
	: m_buffer(s.m_buffer), m_start(s.m_start), m_end(s.m_end)
{
}


stringProxy::stringProxy(const string& s, const size_t start, const size_t end)
	: m_buffer(s), m_start(start),
	  m_end(end == std::numeric_limits <size_t>::max() ? s.length() : end)
{
}


void stringProxy::set(const string& s, const size_t start, const size_t end)
{
	m_buffer = s;
	m_start = start;

	if (end == std::numeric_limits <size_t>::max())
		m_end = s.length();
	else
		m_end = end;
}


void stringProxy::detach()
{
	m_buffer.clear();
	m_start = m_end = 0;
}


stringProxy& stringProxy::operator=(const stringProxy& s)
{
	m_buffer = s.m_buffer;
	m_start = s.m_start;
	m_end = s.m_end;

	return (*this);
}


stringProxy& stringProxy::operator=(const string& s)
{
	m_buffer = s;
	m_start = 0;
	m_end = s.length();

	return (*this);
}


void stringProxy::extract(outputStream& os, const size_t start, const size_t end,
	utility::progressListener* progress) const
{
	size_t len = 0;

	if (end == std::numeric_limits <size_t>::max())
		len = m_end - start - m_start;
	else if (end > start)
		len = end - start;

	if (progress)
		progress->start(len);

	os.write(m_buffer.data() + m_start + start, len);

	if (progress)
	{
		progress->progress(len, len);
		progress->stop(len);
	}
}


size_t stringProxy::length() const
{
	return (m_end - m_start);
}


size_t stringProxy::start() const
{
	return (m_start);
}


size_t stringProxy::end() const
{
	return (m_end);
}


std::ostream& operator<<(std::ostream& os, const stringProxy& s)
{
	outputStreamAdapter adapter(os);
	s.extract(adapter);
	return (os);
}


outputStream& operator<<(outputStream& os, const stringProxy& s)
{
	s.extract(os);
	return (os);
}


} // utility
} // vmime
