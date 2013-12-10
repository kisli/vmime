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

#include "vmime/charsetConverter_idna.hpp"

#include "vmime/exception.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/streamUtils.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


extern "C"
{

#include "contrib/punycode/punycode.h"
#include "contrib/punycode/punycode.c"

}

#include "contrib/utf8/utf8.h"


namespace vmime
{


charsetConverter_idna::charsetConverter_idna
	(const charset& source, const charset& dest, const charsetConverterOptions& opts)
	: m_source(source), m_dest(dest), m_options(opts)
{
}


charsetConverter_idna::~charsetConverter_idna()
{
}


void charsetConverter_idna::convert(utility::inputStream& in, utility::outputStream& out)
{
	// IDNA should be used for short strings, so it does not matter if we
	// do not work directly on the stream
	string inStr;
	vmime::utility::outputStreamStringAdapter os(inStr);
	vmime::utility::bufferedStreamCopy(in, os);

	string outStr;
	convert(inStr, outStr);

	out << outStr;
}


void charsetConverter_idna::convert(const string& in, string& out)
{
	if (m_source == m_dest)
	{
		// No conversion needed
		out = in;
		return;
	}

	out.clear();

	if (m_dest == "idna")
	{
		if (utility::stringUtils::is7bit(in))
		{
			// No need to encode as Punycode
			out = in;
			return;
		}

		string inUTF8;
		charset::convert(in, inUTF8, m_source, vmime::charsets::UTF_8);

		const char* ch = inUTF8.c_str();
		const char* end = inUTF8.c_str() + inUTF8.length();

		std::vector <punycode_uint> unichars;
		unichars.reserve(inUTF8.length());

		while (ch < end)
		{
			const utf8::uint32_t uc = utf8::unchecked::next(ch);
			unichars.push_back(uc);
		}

		std::vector <char> output(inUTF8.length() * 2);
		punycode_uint outputLen = output.size();

		const punycode_status status = punycode_encode
			(unichars.size(), &unichars[0], /* case_flags */ NULL, &outputLen, &output[0]);

		if (status == punycode_success)
		{
			out = string("xn--") + string(output.begin(), output.begin() + outputLen);
		}
		else
		{
			// TODO
		}
	}
	else if (m_source == "idna")
	{
		if (in.length() < 5 || in.substr(0, 4) != "xn--")
		{
			// Not an IDNA string
			out = in;
			return;
		}

		std::vector <punycode_uint> output(in.length() - 4);
		punycode_uint outputLen = output.size();

		const punycode_status status = punycode_decode
			(in.length() - 4, &in[4], &outputLen, &output[0], /* case_flags */ NULL);

		if (status == punycode_success)
		{
			std::vector <char> outUTF8Bytes(outputLen * 4);
			char* p = &outUTF8Bytes[0];

			for (std::vector <punycode_uint>::const_iterator it = output.begin() ;
			     it != output.begin() + outputLen ; ++it)
			{
				p = utf8::unchecked::append(*it, p);
			}

			string outUTF8(&outUTF8Bytes[0], p);
			charset::convert(outUTF8, out, vmime::charsets::UTF_8, m_dest);
		}
		else
		{
			// TODO
		}
	}
}


shared_ptr <utility::charsetFilteredOutputStream> charsetConverter_idna::getFilteredOutputStream(utility::outputStream& /* os */)
{
	return null;
}


} // vmime
