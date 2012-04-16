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

#include "vmime/utility/parserInputStreamAdapter.hpp"


namespace vmime {
namespace utility {


parserInputStreamAdapter::parserInputStreamAdapter(ref <seekableInputStream> stream)
	: m_stream(stream)
{
}


bool parserInputStreamAdapter::eof() const
{
	return m_stream->eof();
}


void parserInputStreamAdapter::reset()
{
	m_stream->reset();
}


stream::size_type parserInputStreamAdapter::read
	(value_type* const data, const size_type count)
{
	return m_stream->read(data, count);
}


ref <seekableInputStream> parserInputStreamAdapter::getUnderlyingStream()
{
	return m_stream;
}


const string parserInputStreamAdapter::extract(const size_type begin, const size_type end) const
{
	const size_type initialPos = m_stream->getPosition();

	try
	{
		value_type *buffer = new value_type[end - begin + 1];

		m_stream->seek(begin);

		const size_type readBytes = m_stream->read(buffer, end - begin);
		buffer[readBytes] = '\0';

		m_stream->seek(initialPos);

		string str(buffer, buffer + readBytes);
		delete [] buffer;

		return str;
	}
	catch (...)
	{
		m_stream->seek(initialPos);
		throw;
	}
}


stream::size_type parserInputStreamAdapter::findNext
	(const std::string& token, const size_type startPosition)
{
	static const unsigned int BUFFER_SIZE = 4096;

	// Token must not be longer than BUFFER_SIZE/2
	if (token.empty() || token.length() > BUFFER_SIZE / 2)
		return npos;

	const size_type initialPos = getPosition();

	seek(startPosition);

	try
	{
		value_type findBuffer[BUFFER_SIZE];
		value_type* findBuffer1 = findBuffer;
		value_type* findBuffer2 = findBuffer + (BUFFER_SIZE / 2) * sizeof(value_type);

		size_type findBufferLen = 0;
		size_type findBufferOffset = 0;

		// Fill in initial buffer
		findBufferLen = read(findBuffer, BUFFER_SIZE * sizeof(value_type));

		for (;;)
		{
			// Find token
			for (value_type *begin = findBuffer, *end = findBuffer + findBufferLen - token.length() ;
			     begin <= end ; ++begin)
			{
				if (begin[0] == token[0] &&
				    (token.length() == 1 ||
				     memcmp(static_cast <const void *>(&begin[1]),
				            static_cast <const void *>(token.data() + 1),
				            token.length() - 1) == 0))
				{
					seek(initialPos);
					return startPosition + findBufferOffset + (begin - findBuffer);
				}
			}

			// Rotate buffer
			memcpy(findBuffer1, findBuffer2, (BUFFER_SIZE / 2) * sizeof(value_type));

			// Read more bytes
			if (findBufferLen < BUFFER_SIZE && eof())
			{
				break;
			}
			else
			{
				const size_type bytesRead = read(findBuffer2, (BUFFER_SIZE / 2) * sizeof(value_type));
				findBufferLen = (BUFFER_SIZE / 2) + bytesRead;
				findBufferOffset += (BUFFER_SIZE / 2);
			}
		}

		seek(initialPos);
	}
	catch (...)
	{
		seek(initialPos);
		throw;
	}

	return npos;
}


} // utility
} // vmime

