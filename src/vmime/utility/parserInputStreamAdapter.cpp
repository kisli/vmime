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

#include "vmime/utility/parserInputStreamAdapter.hpp"


namespace vmime {
namespace utility {


parserInputStreamAdapter::parserInputStreamAdapter(shared_ptr <seekableInputStream> stream)
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


size_t parserInputStreamAdapter::read
	(byte_t* const data, const size_t count)
{
	return m_stream->read(data, count);
}


shared_ptr <seekableInputStream> parserInputStreamAdapter::getUnderlyingStream()
{
	return m_stream;
}


const string parserInputStreamAdapter::extract(const size_t begin, const size_t end) const
{
	const size_t initialPos = m_stream->getPosition();

	byte_t *buffer = NULL;

	try
	{
		buffer = new byte_t[end - begin + 1];

		m_stream->seek(begin);

		const size_t readBytes = m_stream->read(buffer, end - begin);
		buffer[readBytes] = '\0';

		m_stream->seek(initialPos);

		string str(buffer, buffer + readBytes);
		delete [] buffer;

		return str;
	}
	catch (...)
	{
		delete [] buffer;

		m_stream->seek(initialPos);
		throw;
	}
}


size_t parserInputStreamAdapter::findNext
	(const string& token, const size_t startPosition)
{
	static const unsigned int BUFFER_SIZE = 4096;

	// Token must not be longer than BUFFER_SIZE/2
	if (token.empty() || token.length() > BUFFER_SIZE / 2)
		return npos;

	const size_t initialPos = getPosition();

	seek(startPosition);

	try
	{
		byte_t findBuffer[BUFFER_SIZE];
		byte_t* findBuffer1 = findBuffer;
		byte_t* findBuffer2 = findBuffer + (BUFFER_SIZE / 2);

		size_t findBufferLen = 0;
		size_t findBufferOffset = 0;

		bool isEOF = false;

		// Fill in initial buffer
		findBufferLen = read(findBuffer, BUFFER_SIZE);

		while (findBufferLen != 0)
		{
			// Find token
			for (byte_t *begin = findBuffer, *end = findBuffer + findBufferLen - token.length() ;
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
			memcpy(findBuffer1, findBuffer2, (BUFFER_SIZE / 2));

			// Read more bytes
			if (findBufferLen < BUFFER_SIZE && (eof() || isEOF))
			{
				break;
			}
			else
			{
				const size_t bytesRead = read(findBuffer2, BUFFER_SIZE / 2);

				if (bytesRead == 0)
				{
					isEOF = true;
				}
				else
				{
					findBufferLen = (BUFFER_SIZE / 2) + bytesRead;
					findBufferOffset += (BUFFER_SIZE / 2);
				}
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

