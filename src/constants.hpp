//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free SOFTWARE; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software FOUNDATION; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this PROGRAM; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_CONSTANTS_HPP_INCLUDED
#define VMIME_CONSTANTS_HPP_INCLUDED


#include <string>

#include "types.hpp"


namespace vmime
{
	// Media types (predefined types)
	namespace mediaTypes
	{
		// Types
		extern const string::value_type* const TEXT;
		extern const string::value_type* const MULTIPART;
		extern const string::value_type* const MESSAGE;
		extern const string::value_type* const APPLICATION;
		extern const string::value_type* const IMAGE;
		extern const string::value_type* const AUDIO;
		extern const string::value_type* const VIDEO;

		// Sub-types
		extern const string::value_type* const TEXT_PLAIN;
		extern const string::value_type* const TEXT_HTML;
		extern const string::value_type* const TEXT_RICHTEXT;
		extern const string::value_type* const TEXT_ENRICHED;

		extern const string::value_type* const MULTIPART_MIXED;
		extern const string::value_type* const MULTIPART_RELATED;
		extern const string::value_type* const MULTIPART_ALTERNATIVE;
		extern const string::value_type* const MULTIPART_PARALLEL;
		extern const string::value_type* const MULTIPART_DIGEST;

		extern const string::value_type* const MESSAGE_RFC822;
		extern const string::value_type* const MESSAGE_PARTIAL;
		extern const string::value_type* const MESSAGE_EXTERNAL_BODY;

		extern const string::value_type* const APPLICATION_OCTET_STREAM;

		extern const string::value_type* const IMAGE_JPEG;
		extern const string::value_type* const IMAGE_GIF;

		extern const string::value_type* const AUDIO_BASIC;

		extern const string::value_type* const VIDEO_MPEG;
	}


	// Encoding types
	namespace encodingTypes
	{
		extern const string::value_type* const SEVEN_BIT;
		extern const string::value_type* const EIGHT_BIT;
		extern const string::value_type* const BASE64;
		extern const string::value_type* const QUOTED_PRINTABLE;
		extern const string::value_type* const BINARY;
		extern const string::value_type* const UUENCODE;
	}


	// Disposition types (RFC-2183)
	namespace dispositionTypes
	{
		extern const string::value_type* const INLINE;
		extern const string::value_type* const ATTACHMENT;
	}


	// Charsets
	namespace charsets
	{
		extern const string::value_type* const ISO8859_1;
		extern const string::value_type* const ISO8859_2;
		extern const string::value_type* const ISO8859_3;
		extern const string::value_type* const ISO8859_4;
		extern const string::value_type* const ISO8859_5;
		extern const string::value_type* const ISO8859_6;
		extern const string::value_type* const ISO8859_7;
		extern const string::value_type* const ISO8859_8;
		extern const string::value_type* const ISO8859_9;
		extern const string::value_type* const ISO8859_10;
		extern const string::value_type* const ISO8859_13;
		extern const string::value_type* const ISO8859_14;
		extern const string::value_type* const ISO8859_15;
		extern const string::value_type* const ISO8859_16;

		extern const string::value_type* const CP_437;
		extern const string::value_type* const CP_737;
		extern const string::value_type* const CP_775;
		extern const string::value_type* const CP_850;
		extern const string::value_type* const CP_852;
		extern const string::value_type* const CP_853;
		extern const string::value_type* const CP_855;
		extern const string::value_type* const CP_857;
		extern const string::value_type* const CP_858;
		extern const string::value_type* const CP_860;
		extern const string::value_type* const CP_861;
		extern const string::value_type* const CP_862;
		extern const string::value_type* const CP_863;
		extern const string::value_type* const CP_864;
		extern const string::value_type* const CP_865;
		extern const string::value_type* const CP_866;
		extern const string::value_type* const CP_869;
		extern const string::value_type* const CP_874;
		extern const string::value_type* const CP_1125;
		extern const string::value_type* const CP_1250;
		extern const string::value_type* const CP_1251;
		extern const string::value_type* const CP_1252;
		extern const string::value_type* const CP_1253;
		extern const string::value_type* const CP_1254;
		extern const string::value_type* const CP_1255;
		extern const string::value_type* const CP_1256;
		extern const string::value_type* const CP_1257;

		extern const string::value_type* const US_ASCII;

		extern const string::value_type* const UTF_7;
		extern const string::value_type* const UTF_8;
		extern const string::value_type* const UTF_16;
		extern const string::value_type* const UTF_32;

		extern const string::value_type* const WINDOWS_1250;
		extern const string::value_type* const WINDOWS_1251;
		extern const string::value_type* const WINDOWS_1252;
		extern const string::value_type* const WINDOWS_1253;
		extern const string::value_type* const WINDOWS_1254;
		extern const string::value_type* const WINDOWS_1255;
		extern const string::value_type* const WINDOWS_1256;
		extern const string::value_type* const WINDOWS_1257;
		extern const string::value_type* const WINDOWS_1258;
	}
}


#endif // VMIME_CONSTANTS_HPP_INCLUDED
