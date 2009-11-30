//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/stream.hpp"
#include "vmime/utility/stringProxy.hpp"

#include <algorithm>  // for std::copy
#include <iterator>   // for std::back_inserter

#if VMIME_HAVE_MESSAGING_FEATURES
   #include "vmime/net/socket.hpp"
#endif


namespace vmime {
namespace utility {


// stream

stream::size_type stream::getBlockSize()
{
	return 32768;  // 32 KB
}


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


stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os)
{
	return bufferedStreamCopy(is, os, 0, NULL);
}


stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os,
	const stream::size_type length, progressListener* progress)
{
	const stream::size_type blockSize =
		std::min(is.getBlockSize(), os.getBlockSize());

	std::vector <stream::value_type> vbuffer(blockSize);

	stream::value_type* buffer = &vbuffer.front();
	stream::size_type total = 0;

	if (progress != NULL)
		progress->start(length);

	while (!is.eof())
	{
		const stream::size_type read = is.read(buffer, blockSize);

		if (read != 0)
		{
			os.write(buffer, read);
			total += read;

			if (progress != NULL)
				progress->progress(total, std::max(total, length));
		}
	}

	if (progress != NULL)
		progress->stop(total);

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
	m_stream.exceptions(std::ios_base::badbit);
	m_stream.write(data, count);
}


void outputStreamAdapter::flush()
{
	m_stream.exceptions(std::ios_base::badbit);
	m_stream.flush();
}



// outputStreamStringAdapter

outputStreamStringAdapter::outputStreamStringAdapter(string& buffer)
	: m_buffer(buffer)
{
}


void outputStreamStringAdapter::write(const value_type* const data, const size_type count)
{
	m_buffer.append(data, count);
}


void outputStreamStringAdapter::flush()
{
	// Do nothing
}



// outputStreamByteArrayAdapter

outputStreamByteArrayAdapter::outputStreamByteArrayAdapter(byteArray& array)
	: m_array(array)
{
}


void outputStreamByteArrayAdapter::write(const value_type* const data, const size_type count)
{
	m_array.insert(m_array.end(), data, data + count);
}


void outputStreamByteArrayAdapter::flush()
{
	// Do nothing
}



// inputStreamAdapter

inputStreamAdapter::inputStreamAdapter(std::istream& is)
	: m_stream(is)
{
}


bool inputStreamAdapter::eof() const
{
	return (m_stream.eof());
}


void inputStreamAdapter::reset()
{
	m_stream.exceptions(std::ios_base::badbit);
	m_stream.seekg(0, std::ios::beg);
	m_stream.clear();
}


stream::size_type inputStreamAdapter::read
	(value_type* const data, const size_type count)
{
	m_stream.exceptions(std::ios_base::badbit);
	m_stream.read(data, count);
	return (m_stream.gcount());
}


stream::size_type inputStreamAdapter::skip(const size_type count)
{
	m_stream.exceptions(std::ios_base::badbit);
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


bool inputStreamStringAdapter::eof() const
{
	return (m_pos >= m_end);
}


void inputStreamStringAdapter::reset()
{
	m_pos = m_begin;
}


stream::size_type inputStreamStringAdapter::read
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


stream::size_type inputStreamStringAdapter::skip(const size_type count)
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



// inputStreamPointerAdapter

inputStreamPointerAdapter::inputStreamPointerAdapter(std::istream* is, const bool own)
	: m_stream(is), m_own(own)
{
}


inputStreamPointerAdapter::inputStreamPointerAdapter(const inputStreamPointerAdapter&)
	: inputStream(), m_stream(NULL), m_own(false)
{
	// Not copiable
}


inputStreamPointerAdapter::~inputStreamPointerAdapter()
{
	if (m_own)
		delete (m_stream);
}


bool inputStreamPointerAdapter::eof() const
{
	return (m_stream->eof());
}


void inputStreamPointerAdapter::reset()
{
	m_stream->exceptions(std::ios_base::badbit);
	m_stream->seekg(0, std::ios::beg);
	m_stream->clear();
}


stream::size_type inputStreamPointerAdapter::read
	(value_type* const data, const size_type count)
{
	m_stream->exceptions(std::ios_base::badbit);
	m_stream->read(data, count);
	return (m_stream->gcount());
}


stream::size_type inputStreamPointerAdapter::skip(const size_type count)
{
	m_stream->exceptions(std::ios_base::badbit);
	m_stream->ignore(count);
	return (m_stream->gcount());
}



// inputStreamByteBufferAdapter

inputStreamByteBufferAdapter::inputStreamByteBufferAdapter(const byte_t* buffer, const size_type length)
	: m_buffer(buffer), m_length(length), m_pos(0)
{
}


bool inputStreamByteBufferAdapter::eof() const
{
	return m_pos >= m_length;
}


void inputStreamByteBufferAdapter::reset()
{
	m_pos = 0;
}


stream::size_type inputStreamByteBufferAdapter::read
	(value_type* const data, const size_type count)
{
	const size_type remaining = m_length - m_pos;

	if (remaining < count)
	{
		std::copy(m_buffer + m_pos, m_buffer + m_pos + remaining, data);
		m_pos += remaining;

		return remaining;
	}
	else
	{
		std::copy(m_buffer + m_pos, m_buffer + m_pos + count, data);
		m_pos += count;

		return count;
	}
}


stream::size_type inputStreamByteBufferAdapter::skip(const size_type count)
{
	const size_type remaining = m_length - m_pos;

	if (remaining < count)
	{
		m_pos += remaining;
		return remaining;
	}
	else
	{
		m_pos += count;
		return count;
	}
}



#ifdef VMIME_HAVE_MESSAGING_FEATURES


// outputStreamSocketAdapter

outputStreamSocketAdapter::outputStreamSocketAdapter(net::socket& sok)
	: m_socket(sok)
{
}


void outputStreamSocketAdapter::write
	(const value_type* const data, const size_type count)
{
	m_socket.sendRaw(data, count);
}


void outputStreamSocketAdapter::flush()
{
	// Do nothing
}


stream::size_type outputStreamSocketAdapter::getBlockSize()
{
	return m_socket.getBlockSize();
}



// inputStreamSocketAdapter

inputStreamSocketAdapter::inputStreamSocketAdapter(net::socket& sok)
	: m_socket(sok)
{
}


bool inputStreamSocketAdapter::eof() const
{
	// Can't know...
	return false;
}


void inputStreamSocketAdapter::reset()
{
	// Not supported
}


stream::size_type inputStreamSocketAdapter::read
	(value_type* const data, const size_type count)
{
	return m_socket.receiveRaw(data, count);
}


stream::size_type inputStreamSocketAdapter::skip
	(const size_type /* count */)
{
	// Not supported
	return 0;
}


stream::size_type inputStreamSocketAdapter::getBlockSize()
{
	return m_socket.getBlockSize();
}


#endif // VMIME_HAVE_MESSAGING_FEATURES


} // utility
} // vmime
