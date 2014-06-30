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

#ifndef VMIME_WORD_HPP_INCLUDED
#define VMIME_WORD_HPP_INCLUDED


#include "vmime/headerFieldValue.hpp"
#include "vmime/charset.hpp"
#include "vmime/charsetConverterOptions.hpp"


namespace vmime
{


/** A class that encapsulates an encoded-word (RFC-2047):
  * some text encoded into one specified charset.
  */

class VMIME_EXPORT word : public headerFieldValue
{
	friend class text;

public:

	word();
	word(const word& w);
	word(const string& buffer); // Defaults to local charset
	word(const string& buffer, const charset& charset);
	word(const string& buffer, const charset& charset, const string& lang);

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

	/** Return the language used in this word (optional).
	  * If not specified, the value is empty.
	  *
	  * @return language tag for this word, in the format specified
	  * by RFC-1766
	  */
	const string getLanguage() const;

	/** Set the language used in this word (optional).
	  *
	  * @param lang language tag, in the format specified by RFC-1766
	  */
	void setLanguage(const string& lang);

	/** Returns whether two words actually represent the same text,
	  * regardless of their charset.
	  *
	  * @param other word to compare to
	  * @return true if the two words represent the same text, or false otherwise
	  */
	bool isEquivalent(const word& other) const;


	word& operator=(const word& w);
	word& operator=(const string& s);

	bool operator==(const word& w) const;
	bool operator!=(const word& w) const;

	/** Return the contained text converted to the specified charset.
	  *
	  * @param dest output charset
	  * @param opts options for charset conversion
	  * @return word converted to the specified charset
	  */
	const string getConvertedText(const charset& dest,
		const charsetConverterOptions& opts = charsetConverterOptions()) const;

	/** Replace data in this word by data in other word.
	  *
	  * @param other other word to copy data from
	  */
	void copyFrom(const component& other);

	/** Clone this word.
	  *
	  * @return a copy of this word
	  */
	shared_ptr <component> clone() const;


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
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;

public:

	using component::generate;

#ifndef VMIME_BUILDING_DOC
	void generate
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos,
		 size_t* newLinePos,
		 const int flags,
		 generatorState* state) const;
#endif

	const std::vector <shared_ptr <component> > getChildComponents();

private:

	static shared_ptr <word> parseNext
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition,
		 bool prevIsEncoded,
		 bool* isEncoded,
		 bool isFirst);

	static const std::vector <shared_ptr <word> > parseMultiple
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition);


	// The "m_buffer" of this word holds the data, and this data is encoded
	// in the specified "m_charset".
	string m_buffer;
	charset m_charset;
	string m_lang;
};


} // vmime


#endif // VMIME_WORD_HPP_INCLUDED
