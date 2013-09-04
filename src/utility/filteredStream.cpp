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

#include "vmime/utility/filteredStream.hpp"

#include <algorithm>


namespace vmime {
namespace utility {


// filteredInputStream

stream::size_type filteredInputStream::getBlockSize()
{
	return std::min(inputStream::getBlockSize(), getPreviousInputStream().getBlockSize());
}


// filteredOutputStream

stream::size_type filteredOutputStream::getBlockSize()
{
	return std::min(outputStream::getBlockSize(), getNextOutputStream().getBlockSize());
}


// dotFilteredInputStream

dotFilteredInputStream::dotFilteredInputStream(inputStream& is)
	: m_stream(is), m_previousChar2('\0'), m_previousChar1('\0')
{
}


inputStream& dotFilteredInputStream::getPreviousInputStream()
{
	return (m_stream);
}


bool dotFilteredInputStream::eof() const
{
	return (m_stream.eof());
}


void dotFilteredInputStream::reset()
{
	m_previousChar2 = '\0';
	m_previousChar1 = '\0';

	m_stream.reset();
}


stream::size_type dotFilteredInputStream::read(value_type* const data, const size_type count)
{
	const stream::size_type read = m_stream.read(data, count);

	const value_type* readPtr = data;
	value_type* writePtr = data;

	const value_type* end = data + read;

	stream::size_type written = 0;

	// Replace "\n.." with "\n."
	while (readPtr < end)
	{
		if (*readPtr == '.')
		{
			const value_type prevChar2 =
				(readPtr == data + 1 ? m_previousChar1 :
				 readPtr == data ? m_previousChar2 : *(readPtr - 2));
			const value_type prevChar1 =
				(readPtr == data ? m_previousChar1 : *(readPtr - 1));

			if (prevChar2 == '\n' && prevChar1 == '.')
			{
				// Ignore last dot
			}
			else
			{
				*writePtr = *readPtr;

				++writePtr;
				++written;
			}
		}
		else
		{
			*writePtr = *readPtr;

			++writePtr;
			++written;
		}

		++readPtr;
	}

	m_previousChar2 = (read >= 2 ? data[read - 2] : m_previousChar1);
	m_previousChar1 = (read >= 1 ? data[read - 1] : '\0');

	return (written);
}


stream::size_type dotFilteredInputStream::skip(const size_type /* count */)
{
	// Skipping bytes is not supported
	return 0;
}


// dotFilteredOutputStream

dotFilteredOutputStream::dotFilteredOutputStream(outputStream& os)
	: m_stream(os), m_previousChar('\0'), m_start(true)
{
}


outputStream& dotFilteredOutputStream::getNextOutputStream()
{
	return (m_stream);
}


void dotFilteredOutputStream::write
	(const value_type* const data, const size_type count)
{
	if (count == 0)
		return;

	const value_type* pos = data;
	const value_type* end = data + count;
	const value_type* start = data;

	if (m_previousChar == '.')
	{
		if (data[0] == '\n' || data[0] == '\r')
		{
			m_stream.write(".", 1);  // extra <DOT>
			m_stream.write(data, 1);

			pos = data + 1;
		}
	}

	// Replace "\n." with "\n.."
	while ((pos = std::find(pos, end, '.')) != end)
	{
		const value_type previousChar =
			(pos == data ? m_previousChar : *(pos - 1));

		if (previousChar == '\n')
		{
			m_stream.write(start, pos - start);
			m_stream.write("..", 2);

			start = pos + 1;
		}
		else if (pos == data && m_start)  // <DOT><CR><LF> at the beginning of content
		{
			m_stream.write(start, pos - start);

			if (pos + 1 < end && (*(pos + 1) == '\n' || *(pos + 1) == '\r'))
				m_stream.write("..", 2);
			else
				m_stream.write(".", 1);

			start = pos + 1;
		}

		++pos;
	}

	m_stream.write(start, end - start);
	m_previousChar = data[count - 1];
	m_start = false;
}


void dotFilteredOutputStream::flush()
{
	// Do nothing
	m_stream.flush();
}


// CRLFToLFFilteredOutputStream

CRLFToLFFilteredOutputStream::CRLFToLFFilteredOutputStream(outputStream& os)
	: m_stream(os), m_previousChar('\0')
{
}


outputStream& CRLFToLFFilteredOutputStream::getNextOutputStream()
{
	return (m_stream);
}


void CRLFToLFFilteredOutputStream::write
	(const value_type* const data, const size_type count)
{
	if (count == 0)
		return;

	const value_type* pos = data;
	const value_type* end = data + count;
	const value_type* start = data;

	// Warning: if the whole buffer finishes with '\r', this
	// last character will not be written back if flush() is
	// not called
	if (m_previousChar == '\r')
	{
		if (*pos != '\n')
		{
			m_stream.write("\r", 1); // write back \r
			m_previousChar = *pos;
		}
	}

	// Replace "\r\n" (CRLF) with "\n" (LF)
	while ((pos = std::find(pos, end, '\n')) != end)
	{
		const value_type previousChar =
			(pos == data ? m_previousChar : *(pos - 1));

		if (previousChar == '\r')
		{
			if (pos != start)
				m_stream.write(start, pos - 1 - start);  // do not write \r

			m_stream.write("\n", 1);

			start = pos + 1;
		}

		++pos;
	}

	if (data[count - 1] == '\r')
	{
		m_stream.write(start, end - start - 1);
		m_previousChar = '\r';
	}
	else
	{
		m_stream.write(start, end - start);
		m_previousChar = data[count - 1];
	}
}


void CRLFToLFFilteredOutputStream::flush()
{
	m_stream.flush();

	// TODO
}


// LFToCRLFFilteredOutputStream

LFToCRLFFilteredOutputStream::LFToCRLFFilteredOutputStream(outputStream& os)
	: m_stream(os), m_previousChar('\0')
{
}


outputStream& LFToCRLFFilteredOutputStream::getNextOutputStream()
{
	return (m_stream);
}


void LFToCRLFFilteredOutputStream::write
	(const value_type* const data, const size_type count)
{
	if (count == 0)
		return;

	string buffer;
	buffer.reserve(count);

	const value_type* pos = data;
	const value_type* end = data + count;

	value_type previousChar = m_previousChar;

	while (pos < end)
	{
		switch (*pos)
		{
		case '\r':

			buffer.append(1, '\r');
			buffer.append(1, '\n');

			break;

		case '\n':

			if (previousChar != '\r')
			{
				buffer.append(1, '\r');
				buffer.append(1, '\n');
			}

			break;

		default:

			buffer.append(1, *pos);
			break;
		}

		previousChar = *pos;
		++pos;
	}

	m_stream.write(&buffer[0], buffer.length());

	m_previousChar = previousChar;
}


void LFToCRLFFilteredOutputStream::flush()
{
	m_stream.flush();
}


// stopSequenceFilteredInputStream <1>

template <>
stream::size_type stopSequenceFilteredInputStream <1>::read
	(value_type* const data, const size_type count)
{
	if (eof() || m_stream.eof())
	{
		m_eof = true;
		return 0;
	}

	const size_type read = m_stream.read(data, count);
	value_type* end = data + read;

	value_type* pos = std::find(data, end, m_sequence[0]);

	if (pos == end)
	{
		return (read);
	}
	else
	{
		m_found = 1;
		return (pos - data);
	}
}

template <int COUNT>
stream::size_type stopSequenceFilteredInputStream <COUNT>::read
	(value_type* const data, const size_type count)
{
	// Read buffer must be at least 'COUNT' size + 1 byte
	if (eof() || count <= COUNT)
		return 0;

	if (m_stream.eof())
	{
		if (m_found != 0)
		{
			const size_type found = m_found;

			for (size_type f = 0 ; f < found ; ++f)
				data[f] = m_sequence[f];

			m_found = 0;
			m_eof = true;

			return (found);
		}
		else
		{
			m_eof = true;
			return 0;
		}
	}

	size_type read = m_stream.read(data, count - COUNT);

	value_type* end = data + read;
	value_type* pos = data;

	while (pos < end)
	{
		// Very simple case, search for the whole sequence
		if (m_found == 0)
		{
			while (pos < end)
			{
				pos = std::find(pos, end, m_sequence[0]);

				if (pos == end)
					return (read);

				m_found = 1;
				++pos;

				while (pos < end && m_found < COUNT && m_sequence[m_found] == *pos)
				{
					++m_found;
					++pos;
				}

				// Didn't found whole sequence
				if (m_found != COUNT)
				{
					// We reached the end of the buffer
					if (pos == end)
					{
						return (read - m_found);
					}
					// Common prefix but not whole sequence
					else
					{
						m_found = 0;
					}
				}
				// Whole sequence found
				else
				{
					// End of stream
					return (pos - data - m_found);
				}
			}
		}
		// More complex case: search for a sequence which has begun
		// in a previous buffer
		else
		{
			// Search for the end of the previously started sequence
			while (pos < end && m_found < COUNT && m_sequence[m_found] == *pos)
			{
				++m_found;
				++pos;
			}

			if (m_found != COUNT)
			{
				// End of buffer
				if (pos == end)
				{
					// No data: this buffer is a sub-sequence of the
					// searched sequence
					return 0;
				}
				// Common prefix
				else
				{
					// We have to reinject the incomplete sequence into
					// the stream data

					// -- shift right data
					const size_type n = pos - data;

					value_type* newEnd = data + read + m_found - n;
					value_type* oldEnd = data + read;

					for (size_type i = 0 ; i < read - n ; ++i)
					{
						--newEnd;
						--oldEnd;

						*newEnd = *oldEnd;
					}

					// -- copy the prefix just before data
					for (size_type f = 0 ; f < m_found ; ++f)
						data[f] = m_sequence[f];

					read += m_found - n;
					end += m_found - n;

					m_found = 0;
				}
			}
			else
			{
				return 0;  // no more data
			}
		}
	}

	return read;
}

template class stopSequenceFilteredInputStream <5>;
template class stopSequenceFilteredInputStream <3>;

} // utility
} // vmime

