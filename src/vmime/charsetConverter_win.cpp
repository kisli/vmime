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


#if VMIME_CHARSETCONV_LIB_IS_WIN


#include "vmime/charsetConverter_win.hpp"

#include "vmime/exception.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"

#include <string.h>
#include <stdlib.h>


#if (_WIN32 || _WIN64 || WIN32 || WIN64)
	#include <windows.h>
	#include "vmime/platforms/windows/windowsCodepages.hpp"
#else
	#error Please use VMIME_CHARSETCONV_LIB_IS_WIN only on Windows!
#endif


#define CP_UNICODE   1200


namespace vmime
{


// static
shared_ptr <charsetConverter> charsetConverter::createGenericConverter
	(const charset& source, const charset& dest,
	 const charsetConverterOptions& opts)
{
	return make_shared <charsetConverter_win>(source, dest, opts);
}


charsetConverter_win::charsetConverter_win
	(const charset& source, const charset& dest, const charsetConverterOptions& opts)
	: m_source(source), m_dest(dest), m_options(opts)
{
}


void charsetConverter_win::convert(utility::inputStream& in, utility::outputStream& out)
{
	byte_t buffer[32768];
	string inStr, outStr;

	while (!in.eof())
	{
		const size_t len = in.read(buffer, sizeof(buffer));
		utility::stringUtils::appendBytesToString(inStr, buffer, len);
	}

	convert(inStr, outStr);

	out.write(outStr.data(), outStr.length());
}


void charsetConverter_win::convert(const string& in, string& out)
{
	if (m_source == m_dest)
	{
		// No conversion needed
		out = in;
		return;
	}

	const int sourceCodePage = getCodePage(m_source.getName().c_str());
	const int destCodePage = getCodePage(m_dest.getName().c_str());

	// Convert from source charset to Unicode
	std::vector <char> unicodeBuffer;
	const WCHAR* unicodePtr = NULL;
	size_t unicodeLen = 0;

	if (sourceCodePage == CP_UNICODE)
	{
		unicodePtr = reinterpret_cast <const WCHAR*>(in.c_str());
		unicodeLen = in.length() / 2;
	}
	else
	{
		const size_t bufferSize = in.length() * 2;  // in wide characters
		unicodeBuffer.resize(bufferSize);

		unicodePtr = reinterpret_cast <const WCHAR*>(&unicodeBuffer[0]);
		unicodeLen = MultiByteToWideChar
			(sourceCodePage, 0, in.c_str(), static_cast <int>(in.length()),
			 reinterpret_cast <WCHAR*>(&unicodeBuffer[0]), static_cast <int>(bufferSize));
	}

	// Convert from Unicode to destination charset
	if (destCodePage == CP_UNICODE)
	{
		out.assign(reinterpret_cast <const char*>(unicodePtr), unicodeLen * 2);
	}
	else
	{
		const size_t bufferSize = unicodeLen * 6;  // in multibyte characters

		std::vector <char> buffer;
		buffer.resize(bufferSize);

		const size_t len = WideCharToMultiByte
			(destCodePage, 0, unicodePtr, static_cast <int>(unicodeLen),
			 &buffer[0], static_cast <int>(bufferSize), 0, NULL);

		out.assign(&buffer[0], len);
	}
}


// static
int charsetConverter_win::getCodePage(const char* name)
{
	if (_stricmp(name, charsets::UTF_16) == 0)  // wchar_t is UTF-16 on Windows
		return CP_UNICODE;

	// "cp1252" --> return 1252
	if ((name[0] == 'c' || name[0] == 'C') &&
	    (name[1] == 'p' || name[1] == 'P'))
	{
		return atoi(name + 2);
	}

	return vmime::platforms::windows::windowsCodepages::getByName(name);  // throws
}


shared_ptr <utility::charsetFilteredOutputStream>
	charsetConverter_win::getFilteredOutputStream(utility::outputStream& /* os */)
{
	// TODO: implement me!
	return null;
}


} // vmime


#endif // VMIME_CHARSETCONV_LIB_IS_WIN
