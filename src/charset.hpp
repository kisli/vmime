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

#ifndef VMIME_CHARSET_HPP_INCLUDED
#define VMIME_CHARSET_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Charset description (basic type).
  */

class charset : public component
{
public:

	charset();
	charset(const string& name);

public:

	const string name() const { return (m_name); }

	charset& operator=(const charset& source);
	charset& operator=(const string& name);

	const bool operator==(const charset& value) const;
	const bool operator!=(const charset& value) const;

	static const charset getLocaleCharset();

#if VMIME_WIDE_CHAR_SUPPORT
	static void decode(const string& in, wstring& out, const charset& ch);
	static void encode(const wstring& in, string& out, const charset& ch);
#endif

	// In-memory conversion
	static void convert(const string& in, string& out, const charset& source, const charset& dest);

	// Stream conversion
	static void convert(utility::inputStream& in, utility::outputStream& out, const charset& source, const charset& dest);

protected:

	string m_name;

	template <class STRINGF, class STRINGT>
	static void iconvert(const STRINGF& in, STRINGT& out, const charset& from, const charset& to);

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_CHARSET_HPP_INCLUDED
