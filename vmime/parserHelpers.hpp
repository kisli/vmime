//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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
};


} // vmime


#endif // VMIME_PARSERHELPERS_HPP_INCLUDED
