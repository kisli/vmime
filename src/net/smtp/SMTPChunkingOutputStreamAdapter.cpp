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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPChunkingOutputStreamAdapter.hpp"

#include "vmime/net/smtp/SMTPConnection.hpp"
#include "vmime/net/smtp/SMTPTransport.hpp"

#include <algorithm>


namespace vmime {
namespace net {
namespace smtp {


SMTPChunkingOutputStreamAdapter::SMTPChunkingOutputStreamAdapter(ref <SMTPConnection> conn)
	: m_connection(conn), m_bufferSize(0), m_chunkCount(0)
{
}


void SMTPChunkingOutputStreamAdapter::sendChunk
	(const value_type* const data, const size_type count, const bool last)
{
	if (count == 0 && !last)
	{
		// Nothing to send
		return;
	}

	// Send this chunk
	m_connection->sendRequest(SMTPCommand::BDAT(count, last));
	m_connection->getSocket()->sendRaw(data, count);

	++m_chunkCount;

	// If PIPELINING is not supported, read one response for this BDAT command
	if (!m_connection->hasExtension("PIPELINING"))
	{
		ref <SMTPResponse> resp = m_connection->readResponse();

		if (resp->getCode() != 250)
		{
			m_connection->getTransport()->disconnect();
			throw exceptions::command_error("BDAT", resp->getText());
		}
	}
	// If PIPELINING is supported, read one response for each chunk (ie. number
	// of BDAT commands issued) after the last chunk has been sent
	else if (last)
	{
		bool invalidReply = false;
		ref <SMTPResponse> resp;

		for (unsigned int i = 0 ; i < m_chunkCount ; ++i)
		{
			resp = m_connection->readResponse();

			if (resp->getCode() != 250)
				invalidReply = true;
		}

		if (invalidReply)
		{
			m_connection->getTransport()->disconnect();
			throw exceptions::command_error("BDAT", resp->getText());
		}
	}
}


void SMTPChunkingOutputStreamAdapter::write
	(const value_type* const data, const size_type count)
{
	const value_type* curData = data;
	size_type curCount = count;

	while (curCount != 0)
	{
		// Fill the buffer
		const size_type remaining = sizeof(m_buffer) - m_bufferSize;
		const size_type bytesToCopy = std::min(remaining, curCount);

		std::copy(data, data + bytesToCopy, m_buffer);

		m_bufferSize += bytesToCopy;
		curData += bytesToCopy;
		curCount -= bytesToCopy;

		// If the buffer is full, send this chunk
		if (m_bufferSize >= sizeof(m_buffer))
		{
			sendChunk(m_buffer, m_bufferSize, /* last */ false);
			m_bufferSize = 0;
		}
	}
}


void SMTPChunkingOutputStreamAdapter::flush()
{
	sendChunk(m_buffer, m_bufferSize, /* last */ true);
	m_bufferSize = 0;
}


utility::stream::size_type SMTPChunkingOutputStreamAdapter::getBlockSize()
{
	return sizeof(m_buffer);
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
