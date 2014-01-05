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

size_t filteredInputStream::getBlockSize()
{
	return std::min(inputStream::getBlockSize(), getPreviousInputStream().getBlockSize());
}


// filteredOutputStream

size_t filteredOutputStream::getBlockSize()
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


size_t dotFilteredInputStream::read(byte_t* const data, const size_t count)
{
	const size_t read = m_stream.read(data, count);

	const byte_t* readPtr = data;
	byte_t* writePtr = data;

	const byte_t* end = data + read;

	size_t written = 0;

	// Replace "\n.." with "\n."
	while (readPtr < end)
	{
		if (*readPtr == '.')
		{
			const byte_t prevChar2 =
				(readPtr == data + 1 ? m_previousChar1 :
				 readPtr == data ? m_previousChar2 : *(readPtr - 2));
			const byte_t prevChar1 =
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


size_t dotFilteredInputStream::skip(const size_t /* count */)
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


void dotFilteredOutputStream::writeImpl
	(const byte_t* const data, const size_t count)
{
	if (count == 0)
		return;

	const byte_t* pos = data;
	const byte_t* end = data + count;
	const byte_t* start = data;

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
		const byte_t previousChar =
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


size_t dotFilteredOutputStream::getBlockSize()
{
	return m_stream.getBlockSize();
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


void CRLFToLFFilteredOutputStream::writeImpl
	(const byte_t* const data, const size_t count)
{
	if (count == 0)
		return;

	const byte_t* pos = data;
	const byte_t* end = data + count;
	const byte_t* start = data;

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
		const byte_t previousChar =
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


size_t CRLFToLFFilteredOutputStream::getBlockSize()
{
	return m_stream.getBlockSize();
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


void LFToCRLFFilteredOutputStream::writeImpl
	(const byte_t* const data, const size_t count)
{
	if (count == 0)
		return;

	string buffer;
	buffer.reserve(count);

	const byte_t* pos = data;
	const byte_t* end = data + count;

	byte_t previousChar = m_previousChar;

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


size_t LFToCRLFFilteredOutputStream::getBlockSize()
{
	return m_stream.getBlockSize();
}


// stopSequenceFilteredInputStream <1>

template <>
size_t stopSequenceFilteredInputStream <1>::read
	(byte_t* const data, const size_t count)
{
	if (eof() || m_stream.eof())
	{
		m_eof = true;
		return 0;
	}

	const size_t read = m_stream.read(data, count);
	byte_t* end = data + read;

	byte_t* pos = std::find(data, end, m_sequence[0]);

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


} // utility
} // vmime

