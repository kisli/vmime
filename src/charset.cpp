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

#include "vmime/charset.hpp"
#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include "vmime/utility/stringUtils.hpp"


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


charset::charset()
	: m_name(charsets::US_ASCII)
{
}


charset::charset(const string& name)
	: m_name(name)
{
}


charset::charset(const char* name)
	: m_name(name)
{
}


void charset::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = string(buffer.begin() + position, buffer.begin() + end);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void charset::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


void charset::convert(utility::inputStream& in, utility::outputStream& out,
	const charset& source, const charset& dest)
{
	// Get an iconv descriptor
	const iconv_t cd = iconv_open(dest.getName().c_str(), source.getName().c_str());

	if (cd != reinterpret_cast <iconv_t>(-1))
	{
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

		// Close iconv handle
		iconv_close(cd);
	}
	else
	{
		throw exceptions::charset_conv_error();
	}
}


template <class STRINGF, class STRINGT>
void charset::iconvert(const STRINGF& in, STRINGT& out, const charset& from, const charset& to)
{
	// Get an iconv descriptor
	const iconv_t cd = iconv_open(to.getName().c_str(), from.getName().c_str());

	typedef typename STRINGF::value_type ivt;
	typedef typename STRINGT::value_type ovt;

	if (cd != reinterpret_cast <iconv_t>(-1))
	{
		out.clear();

		char buffer[65536];

		const char* inBuffer = static_cast <const char*>(in.data());
		size_t inBytesLeft = in.length();

		for ( ; inBytesLeft > 0 ; )
		{
			size_t outBytesLeft = sizeof(buffer);
			char* outBuffer = buffer;

			if (iconv(cd, ICONV_HACK(&inBuffer), &inBytesLeft,
			              &outBuffer, &outBytesLeft) == static_cast <size_t>(-1))
			{
				out += STRINGT(static_cast <ovt*>(buffer), sizeof(buffer) - outBytesLeft);

				// Ignore this "blocking" character and continue
				out += '?';
				++inBuffer;
				--inBytesLeft;
			}
			else
			{
				out += STRINGT(static_cast <ovt*>(buffer), sizeof(buffer) - outBytesLeft);
			}
		}

		// Close iconv handle
		iconv_close(cd);
	}
	else
	{
		throw exceptions::charset_conv_error();
	}
}


#if VMIME_WIDE_CHAR_SUPPORT

void charset::decode(const string& in, wstring& out, const charset& ch)
{
	iconvert(in, out, ch, charset("WCHAR_T"));
}


void charset::encode(const wstring& in, string& out, const charset& ch)
{
	iconvert(in, out, charset("WCHAR_T"), ch);
}

#endif


void charset::convert(const string& in, string& out, const charset& source, const charset& dest)
{
	iconvert(in, out, source, dest);
}


const charset charset::getLocaleCharset()
{
	return (platformDependant::getHandler()->getLocaleCharset());
}


charset& charset::operator=(const charset& other)
{
	copyFrom(other);
	return (*this);
}


const bool charset::operator==(const charset& value) const
{
	return (utility::stringUtils::isStringEqualNoCase(m_name, value.m_name));
}


const bool charset::operator!=(const charset& value) const
{
	return !(*this == value);
}


ref <component> charset::clone() const
{
	return vmime::create <charset>(m_name);
}


const string& charset::getName() const
{
	return (m_name);
}


void charset::copyFrom(const component& other)
{
	m_name = dynamic_cast <const charset&>(other).m_name;
}


const std::vector <ref <const component> > charset::getChildComponents() const
{
	return std::vector <ref <const component> >();
}


} // vmime
