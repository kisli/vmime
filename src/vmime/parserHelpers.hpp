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

#ifndef VMIME_PARSERHELPERS_HPP_INCLUDED
#define VMIME_PARSERHELPERS_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/utility/stringUtils.hpp"

#include <algorithm>



namespace vmime
{


class parserHelpers
{
public:

	static bool isSpace(const char_t c)
	{
		return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
	}

	static bool isSpaceOrTab(const char_t c)
	{
		return (c == ' ' || c == '\t');
	}

	static bool isDigit(const char_t c)
	{
		return (c >= '0' && c <= '9');
	}


	static bool isAlpha(const char_t c)
	{
		return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
	}


	static char_t toLower(const char_t c)
	{
		if (c >= 'A' && c <= 'Z')
			return ('a' + (c - 'A'));
		else
			return c;
	}


	// Checks whether a character is in the 7-bit US-ASCII charset

	static bool isAscii(const char_t c)
	{
		const unsigned int x = static_cast <unsigned int>(c);
		return (x <= 127);
	}


	// Checks whether a character has a visual representation

	static bool isPrint(const char_t c)
	{
		const unsigned int x = static_cast <unsigned int>(c);
		return (x >= 0x20 && x <= 0x7E);
	}


	/** Finds the next EOL sequence in the specified buffer.
	  * An EOL sequence may be a CR+LF sequence, or a LF sequence.
	  *
	  * @param buffer search buffer
	  * @param currentPos start searching from this position
	  * @param end stop searching at this position
	  * @param eol will receive the position after the EOL sequence
	  * @return true if an EOL sequence has been found, or false if
	  * no EOL sequence was found before the end of the buffer
	  */
	static bool findEOL(const string& buffer, const size_t currentPos, const size_t end, size_t* eol)
	{
		size_t pos = currentPos;

		if (pos == end)
			return false;

		while (pos < end)
		{
			if (buffer[pos] == '\r' && pos + 1 < end && buffer[pos + 1] == '\n')
			{
				*eol = pos + 2;
				return true;
			}
			else if (buffer[pos] == '\n')
			{
				*eol = pos + 1;
				return true;
			}

			++pos;
		}

		*eol = end;

		return true;
	}
};


} // vmime


#endif // VMIME_PARSERHELPERS_HPP_INCLUDED
