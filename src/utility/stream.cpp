//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/utility/stream.hpp"
#include "vmime/utility/stringProxy.hpp"

#include <algorithm>  // for std::copy
#include <iterator>   // for std::back_inserter


namespace vmime {
namespace utility {


// Helpers

outputStream& operator<<(outputStream& os, const stream::value_type c)
{
	os.write(&c, 1);
	return (os);
}


outputStream& operator<<(outputStream& os, const string& str)
{
	os.write(str.data(), str.length());
	return (os);
}


const stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os)
{
	stream::value_type buffer[65536];
	stream::size_type total = 0;

	while (!is.eof())
	{
		const stream::size_type read = is.read(buffer, sizeof(buffer));

		if (read != 0)
		{
			os.write(buffer, read);
			total += read;
		}
	}

	return (total);
}



// outputStreamAdapter

outputStreamAdapter::outputStreamAdapter(std::ostream& os)
	: m_stream(os)
{
}


void outputStreamAdapter::write
	(const value_type* const data, const size_type count)
{
	m_stream.write(data, count);
}



// outputStreamStringAdapter

outputStreamStringAdapter::outputStreamStringAdapter(string& buffer)
	: m_buffer(buffer)
{
	m_buffer.clear();
}


void outputStreamStringAdapter::write(const value_type* const data, const size_type count)
{
	// TODO: better way?
	std::copy(data, data + count, std::back_inserter(m_buffer));
}



// inputStreamAdapter

inputStreamAdapter::inputStreamAdapter(std::istream& is)
	: m_stream(is)
{
}


const bool inputStreamAdapter::eof() const
{
	return (m_stream.eof());
}


void inputStreamAdapter::reset()
{
	m_stream.seekg(0, std::ios::beg);
	m_stream.clear();
}


const stream::size_type inputStreamAdapter::read
	(value_type* const data, const size_type count)
{
	m_stream.read(data, count);
	return (m_stream.gcount());
}


const stream::size_type inputStreamAdapter::skip(const size_type count)
{
	m_stream.ignore(count);
	return (m_stream.gcount());
}



// inputStreamStringAdapter

inputStreamStringAdapter::inputStreamStringAdapter(const string& buffer)
	: m_buffer(buffer), m_begin(0), m_end(buffer.length()), m_pos(0)
{
}


inputStreamStringAdapter::inputStreamStringAdapter(const string& buffer,
	const string::size_type begin, const string::size_type end)
	: m_buffer(buffer), m_begin(begin), m_end(end), m_pos(begin)
{
}


const bool inputStreamStringAdapter::eof() const
{
	return (m_pos >= m_end);
}


void inputStreamStringAdapter::reset()
{
	m_pos = m_begin;
}


const stream::size_type inputStreamStringAdapter::read
	(value_type* const data, const size_type count)
{
	if (m_pos + count >= m_end)
	{
		const size_type remaining = m_end - m_pos;

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


const stream::size_type inputStreamStringAdapter::skip(const size_type count)
{
	if (m_pos + count >= m_end)
	{
		const size_type remaining = m_end - m_pos;
		m_pos = m_end;
		return (remaining);
	}
	else
	{
		m_pos += count;
		return (count);
	}
}



// inputStreamStringProxyAdapter

inputStreamStringProxyAdapter::inputStreamStringProxyAdapter(const stringProxy& buffer)
	: m_buffer(buffer), m_pos(0)
{
}


const bool inputStreamStringProxyAdapter::eof() const
{
	return (m_pos >= m_buffer.length());
}


void inputStreamStringProxyAdapter::reset()
{
	m_pos = 0;
}


const stream::size_type inputStreamStringProxyAdapter::read
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


const stream::size_type inputStreamStringProxyAdapter::skip(const size_type count)
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



// inputStreamPointerAdapter

inputStreamPointerAdapter::inputStreamPointerAdapter(std::istream* is, const bool own)
	: m_stream(is), m_own(own)
{
}


inputStreamPointerAdapter::~inputStreamPointerAdapter()
{
	if (m_own)
		delete (m_stream);
}


const bool inputStreamPointerAdapter::eof() const
{
	return (m_stream->eof());
}


void inputStreamPointerAdapter::reset()
{
	m_stream->seekg(0, std::ios::beg);
	m_stream->clear();
}


const stream::size_type inputStreamPointerAdapter::read
	(value_type* const data, const size_type count)
{
	m_stream->read(data, count);
	return (m_stream->gcount());
}


const stream::size_type inputStreamPointerAdapter::skip(const size_type count)
{
	m_stream->ignore(count);
	return (m_stream->gcount());
}


} // utility
} // vmime
