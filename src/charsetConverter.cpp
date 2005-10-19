//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/charsetConverter.hpp"
#include "vmime/exception.hpp"


extern "C"
{
#ifndef VMIME_BUILDING_DOC

	#include <iconv.h>
	#include <errno.h>

	// HACK: prototypes may differ depending on the compiler and/or system (the
	// second parameter may or may not be 'const'). This redeclaration is a hack
	// to have a common prototype "iconv_cast".
	class ICONV_HACK
	{
	public:

		ICONV_HACK(const char** ptr) : m_ptr(ptr) { }

		operator const char**() { return m_ptr; }
		operator char**() { return const_cast <char**>(m_ptr); }

	private:

		const char** m_ptr;
	};

#endif // VMIME_BUILDING_DOC
}


namespace vmime
{


charsetConverter::charsetConverter(const charset& source, const charset& dest)
	: m_desc(NULL), m_source(source), m_dest(dest)
{
	// Get an iconv descriptor
	const iconv_t cd = iconv_open(dest.getName().c_str(), source.getName().c_str());

	if (cd != reinterpret_cast <iconv_t>(-1))
	{
		iconv_t* p = new iconv_t;
		*p= cd;

		m_desc = p;
	}
}


charsetConverter::~charsetConverter()
{
	if (m_desc != NULL)
	{
		// Close iconv handle
		iconv_close(*static_cast <iconv_t*>(m_desc));

		delete static_cast <iconv_t*>(m_desc);
	}
}


void charsetConverter::convert(utility::inputStream& in, utility::outputStream& out)
{
	if (m_desc == NULL)
		throw exceptions::charset_conv_error("Cannot initialize converter.");

	const iconv_t cd = *static_cast <iconv_t*>(m_desc);

	char inBuffer[32768];
	char outBuffer[32768];
	size_t inPos = 0;

	bool prevIsInvalid = false;

	while (true)
	{
		// Fullfill the buffer
		size_t inLength = static_cast <size_t>(in.read(inBuffer + inPos, sizeof(inBuffer) - inPos) + inPos);
		size_t outLength = sizeof(outBuffer);

		const char* inPtr = inBuffer;
		char* outPtr = outBuffer;

		// Convert input bytes
		if (iconv(cd, ICONV_HACK(&inPtr), &inLength,
			      &outPtr, &outLength) == static_cast <size_t>(-1))
		{
			// Illegal input sequence or input sequence has no equivalent
			// sequence in the destination charset.
			if (prevIsInvalid)
			{
				// Write successfully converted bytes
				out.write(outBuffer, sizeof(outBuffer) - outLength);

				// Output a special character to indicate we don't known how to
				// convert the sequence at this position
				out.write("?", 1);

				// Skip a byte and leave unconverted bytes in the input buffer
				std::copy(const_cast <char*>(inPtr + 1), inBuffer + sizeof(inBuffer), inBuffer);
				inPos = inLength - 1;
			}
			else
			{
				// Write successfully converted bytes
				out.write(outBuffer, sizeof(outBuffer) - outLength);

				// Leave unconverted bytes in the input buffer
				std::copy(const_cast <char*>(inPtr), inBuffer + sizeof(inBuffer), inBuffer);
				inPos = inLength;

				if (errno != E2BIG)
					prevIsInvalid = true;
			}
		}
		else
		{
			// Write successfully converted bytes
			out.write(outBuffer, sizeof(outBuffer) - outLength);

			inPos = 0;
			prevIsInvalid = false;
		}

		// Check for end of data
		if (in.eof() && inPos == 0)
			break;
	}
}


void charsetConverter::convert(const string& in, string& out)
{
	utility::inputStreamStringAdapter is(in);
	utility::outputStreamStringAdapter os(out);

	convert(is, os);
}


} // vmime
