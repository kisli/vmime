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

#ifndef VMIME_PARSERHELPERS_HPP_INCLUDED
#define VMIME_PARSERHELPERS_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/utility/stringUtils.hpp"

#include <algorithm>



namespace vmime
{


inline const bool isspace(const char_t c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}


inline const bool isdigit(const char_t c)
{
	return (c >= '0' && c <= '9');
}


inline const bool isalpha(const char_t c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}


inline const char_t tolower(const char_t c)
{
	if (c >= 'A' && c <= 'Z')
		return ('a' + (c - 'A'));
	else
		return c;
}


// Checks whether a character is in the 7-bit US-ASCII charset

inline const bool isascii(const char_t c)
{
	return (c <= 127);
}


// Checks whether a character has a visual representation

inline const bool isprint(const char_t c)
{
	return (c >= 0x20 && c <= 0x7E);
}


} // vmime


#endif // VMIME_PARSERHELPERS_HPP_INCLUDED
