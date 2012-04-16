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

#ifndef VMIME_WORD_HPP_INCLUDED
#define VMIME_WORD_HPP_INCLUDED


#include "vmime/headerFieldValue.hpp"
#include "vmime/charset.hpp"


namespace vmime
{


/** A class that encapsulates an encoded-word (RFC-2047):
  * some text encoded into one specified charset.
  */

class word : public headerFieldValue
{
	friend class text;

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

	/** Tests whether this word is empty.
	  *
	  * @return true if the buffer is empty, false otherwise
	  */
	bool isEmpty() const;

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

	bool operator==(const word& w) const;
	bool operator!=(const word& w) const;

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
	void copyFrom(const component& other);

	/** Clone this word.
	  *
	  * @return a copy of this word
	  */
	ref <component> clone() const;


#ifndef VMIME_BUILDING_DOC
	class generatorState
	{
	public:

		generatorState()
			: isFirstWord(true), prevWordIsEncoded(false), lastCharIsSpace(false)
		{
		}

		bool isFirstWord;
		bool prevWordIsEncoded;
		bool lastCharIsSpace;
	};
#endif


protected:

	void parseImpl
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	void generateImpl
		(utility::outputStream& os,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;

public:

	using component::generate;

#ifndef VMIME_BUILDING_DOC
	void generate
		(utility::outputStream& os,
		 const string::size_type maxLineLength,
		 const string::size_type curLinePos,
		 string::size_type* newLinePos,
		 const int flags,
		 generatorState* state) const;
#endif

	const std::vector <ref <component> > getChildComponents();

private:

	static ref <word> parseNext
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition,
		 bool prevIsEncoded,
		 bool* isEncoded,
		 bool isFirst);

	static const std::vector <ref <word> > parseMultiple
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition);


	// The "m_buffer" of this word holds the data, and this data is encoded
	// in the specified "m_charset".
	string m_buffer;
	charset m_charset;
};


} // vmime


#endif // VMIME_WORD_HPP_INCLUDED
