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

#ifndef VMIME_WORD_HPP_INCLUDED
#define VMIME_WORD_HPP_INCLUDED


#include "charset.hpp"


namespace vmime
{


/** A class that encapsulates an encoded-word (RFC-2047):
  * some text encoded into one specified charset.
  */

class word
{
public:

	word();
	word(const word& w);
	word(const string& buffer); // Defaults to locale charset
	word(const string& buffer, const class charset& charset);

	const string& buffer() const { return (m_buffer); }
	string& buffer() { return (m_buffer); }

	const class charset& charset() const { return (m_charset); }
	class charset& charset() { return (m_charset); }


	word& operator=(const word& w);
	word& operator=(const string& s);

	const bool operator==(const word& w) const;
	const bool operator!=(const word& w) const;

#if VMIME_WIDE_CHAR_SUPPORT
	const wstring getDecodedText() const;
#endif
	const string getConvertedText(const class charset& dest) const;

protected:

	// The "m_buffer" of this word holds the data, and this data is encoded
	// in the specified "m_charset".
	string m_buffer;
	class charset m_charset;
};


} // vmime


#endif // VMIME_WORD_HPP_INCLUDED
