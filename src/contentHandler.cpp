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

#include "contentHandler.hpp"


namespace vmime
{


// No encoding = "binary" encoding
const encoding contentHandler::NO_ENCODING(encodingTypes::BINARY);


contentHandler::contentHandler()
	: m_type(TYPE_NONE), m_encoding(NO_ENCODING), m_ownedStream(NULL), m_stream(NULL)
{
}


contentHandler::contentHandler(const string& buffer, const vmime::encoding& enc)
	: m_type(TYPE_STRING), m_encoding(enc), m_string(buffer),
	  m_ownedStream(NULL), m_stream(NULL)
{
}


contentHandler::~contentHandler()
{
}


contentHandler::contentHandler(const contentHandler& cts)
	: m_type(cts.m_type), m_encoding(cts.m_encoding), m_string(cts.m_string),
	  m_ownedStream(const_cast <utility::smart_ptr <utility::inputStream>&>(cts.m_ownedStream)),
	  m_stream(cts.m_stream), m_length(cts.m_length)
{
}


contentHandler& contentHandler::operator=(const contentHandler& cts)
{
	m_type = cts.m_type;
	m_encoding = cts.m_encoding;

	m_string = cts.m_string;

	m_ownedStream = const_cast <utility::smart_ptr <utility::inputStream>&>(cts.m_ownedStream);
	m_stream = cts.m_stream;
	m_length = cts.m_length;

	return (*this);
}


void contentHandler::set(const utility::stringProxy& str, const vmime::encoding& enc)
{
	m_type = TYPE_STRING;
	m_encoding = enc;

	m_string = str;

	m_ownedStream = NULL;
	m_stream = NULL;
}


void contentHandler::set(const string& buffer, const vmime::encoding& enc)
{
	m_type = TYPE_STRING;
	m_encoding = enc;

	m_string.set(buffer);

	m_ownedStream = NULL;
	m_stream = NULL;
}


void contentHandler::set(const string& buffer, const string::size_type start,
	const string::size_type end, const vmime::encoding& enc)
{
	m_type = TYPE_STRING;
	m_encoding = enc;

	m_string.set(buffer, start, end);

	m_ownedStream = NULL;
	m_stream = NULL;
}


void contentHandler::set(utility::inputStream* const is, const string::size_type length,
	const bool own, const vmime::encoding& enc)
{
	m_type = TYPE_STREAM;
	m_encoding = enc;

	m_length = length;

	if (own)
	{
		m_ownedStream = is;
		m_stream = NULL;
	}
	else
	{
		m_ownedStream = NULL;
		m_stream = is;
	}

	m_string.detach();
}


contentHandler& contentHandler::operator=(const string& buffer)
{
	set(buffer, NO_ENCODING);
	return (*this);
}


void contentHandler::generate(utility::outputStream& os, const vmime::encoding& enc,
	const string::size_type maxLineLength) const
{
	if (m_type == TYPE_NONE)
		return;

	// Managed data is already encoded
	if (isEncoded())
	{
		// The data is already encoded but the encoding specified for
		// the generation is different from the current one. We need
		// to re-encode data: decode from input buffer to temporary
		// buffer, and then re-encode to output stream...
		if (m_encoding != enc)
		{
			utility::auto_ptr <encoder> theDecoder(m_encoding.getEncoder());
			utility::auto_ptr <encoder> theEncoder(enc.getEncoder());

			theEncoder->properties()["maxlinelength"] = maxLineLength;

			switch (m_type)
			{
			default:
			{
				// No data
				break;
			}
			case TYPE_STRING:
			{
				utility::inputStreamStringProxyAdapter in(m_string);

				std::ostringstream oss;
				utility::outputStreamAdapter tempOut(oss);

				theDecoder->decode(in, tempOut);

				string str = oss.str();
				utility::inputStreamStringAdapter tempIn(str);

				theEncoder->encode(tempIn, os);

				break;
			}
			case TYPE_STREAM:
			{
				utility::inputStream& in = const_cast <utility::inputStream&>
					(*(m_stream ? m_stream : m_ownedStream.ptr()));

				in.reset();  // may not work...

				std::ostringstream oss;
				utility::outputStreamAdapter tempOut(oss);

				theDecoder->decode(in, tempOut);

				string str = oss.str();
				utility::inputStreamStringAdapter tempIn(str);

				theEncoder->encode(tempIn, os);

				break;
			}

			}
		}
		// No encoding to perform
		else
		{
			switch (m_type)
			{
			default:
			{
				// No data
				break;
			}
			case TYPE_STRING:
			{
				m_string.extract(os);
				break;
			}
			case TYPE_STREAM:
			{
				utility::inputStream& in = const_cast <utility::inputStream&>
					(*(m_stream ? m_stream : m_ownedStream.ptr()));

				in.reset();  // may not work...

				utility::bufferedStreamCopy(in, os);
				break;
			}

			}
		}
	}
	// Need to encode data before
	else
	{
		utility::auto_ptr <encoder> theEncoder(enc.getEncoder());
		theEncoder->properties()["maxlinelength"] = maxLineLength;

		// Encode the contents
		switch (m_type)
		{
		default:
		{
			// No data
			break;
		}
		case TYPE_STRING:
		{
			utility::inputStreamStringProxyAdapter in(m_string);

			theEncoder->encode(in, os);
			break;
		}
		case TYPE_STREAM:
		{
			utility::inputStream& in = const_cast <utility::inputStream&>
				(*(m_stream ? m_stream : m_ownedStream.ptr()));

			in.reset();  // may not work...

			theEncoder->encode(in, os);
			break;
		}

		}
	}
}


void contentHandler::extract(utility::outputStream& os) const
{
	if (m_type == TYPE_NONE)
		return;

	// No decoding to perform
	if (!isEncoded())
	{
		switch (m_type)
		{
		default:
		{
			// No data
			break;
		}
		case TYPE_STRING:
		{
			m_string.extract(os);
			break;
		}
		case TYPE_STREAM:
		{
			utility::inputStream& in = const_cast <utility::inputStream&>
				(*(m_stream ? m_stream : m_ownedStream.ptr()));

			in.reset();  // may not work...

			utility::bufferedStreamCopy(in, os);
			break;
		}

		}
	}
	// Need to decode data
	else
	{
		utility::auto_ptr <encoder> theDecoder(m_encoding.getEncoder());

		switch (m_type)
		{
		default:
		{
			// No data
			break;
		}
		case TYPE_STRING:
		{
			utility::inputStreamStringProxyAdapter in(m_string);

			theDecoder->decode(in, os);
			break;
		}
		case TYPE_STREAM:
		{
			utility::inputStream& in = const_cast <utility::inputStream&>
				(*(m_stream ? m_stream : m_ownedStream.ptr()));

			in.reset();  // may not work...

			theDecoder->decode(in, os);
			break;
		}

		}
	}
}


const string::size_type contentHandler::length() const
{
	switch (m_type)
	{
	case TYPE_NONE: return (0);
	case TYPE_STRING: return (m_string.length());
	case TYPE_STREAM: return (m_length);
	}

	return (0);
}


const bool contentHandler::empty() const
{
	return (m_type == TYPE_NONE);
}


const bool contentHandler::isEncoded() const
{
	return (m_encoding != NO_ENCODING);
}


const vmime::encoding& contentHandler::encoding() const
{
	return (m_encoding);
}


} // vmime
