//
// VMime library (http://vmime.sourceforge.net)
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_WORD_HPP_INCLUDED
#define VMIME_WORD_HPP_INCLUDED


#include "vmime/charset.hpp"


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
	word(const string& buffer, const charset& charset);

	/** Return the raw data for this encoded word.
	  *
	  * @return raw data buffer
	  */
	const string& getBuffer() const;

	/** Return the raw data for this encoded word.
	  *
	  * @return raw data buffer
	  */
	string& getBuffer();

	/** Set the raw data for this encoded word.
	  *
	  * @param buffer raw data buffer
	  */
	void setBuffer(const string& buffer);

	/** Return the charset of this word.
	  *
	  * @return charset for this word
	  */
	const charset& getCharset() const;

	/** Set the charset of this word.
	  *
	  * @param ch charset of this word
	  */
	void setCharset(const charset& ch);


	word& operator=(const word& w);
	word& operator=(const string& s);

	const bool operator==(const word& w) const;
	const bool operator!=(const word& w) const;

#if VMIME_WIDE_CHAR_SUPPORT
	const wstring getDecodedText() const;
#endif

	/** Return the contained text converted to the specified charset.
	  *
	  * @param dest output charset
	  * @return word converted to the specified charset
	  */
	const string getConvertedText(const charset& dest) const;

	/** Replace data in this word by data in other word.
	  *
	  * @param other other word to copy data from
	  */
	void copyFrom(const word& other);

	/** Clone this word.
	  *
	  * @return a copy of this word
	  */
	word* clone() const;

private:

	// The "m_buffer" of this word holds the data, and this data is encoded
	// in the specified "m_charset".
	string m_buffer;
	charset m_charset;
};


} // vmime


#endif // VMIME_WORD_HPP_INCLUDED
