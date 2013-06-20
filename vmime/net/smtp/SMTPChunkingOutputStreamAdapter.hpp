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

#ifndef VMIME_NET_SMTP_SMTPCHUNKINGOUTPUTSTREAMADAPTER_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPCHUNKINGOUTPUTSTREAMADAPTER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/utility/outputStream.hpp"


namespace vmime {
namespace net {
namespace smtp {


class SMTPConnection;


/** An output stream adapter used with ESMTP CHUNKING extension.
  */
class VMIME_EXPORT SMTPChunkingOutputStreamAdapter : public utility::outputStream
{
	friend class vmime::creator;

public:

	SMTPChunkingOutputStreamAdapter(ref <SMTPConnection> conn);

	void write(const value_type* const data, const size_type count);
	void flush();

	size_type getBlockSize();

private:

	SMTPChunkingOutputStreamAdapter(const SMTPChunkingOutputStreamAdapter&);


	void sendChunk(const value_type* const data, const size_type count, const bool last);


	ref <SMTPConnection> m_connection;

	value_type m_buffer[262144];  // 256 KB
	size_type m_bufferSize;

	unsigned int m_chunkCount;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPCHUNKINGOUTPUTSTREAMADAPTER_HPP_INCLUDED
