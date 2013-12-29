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

#ifndef VMIME_ENCODING_HPP_INCLUDED
#define VMIME_ENCODING_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"

#include "vmime/utility/encoder/encoder.hpp"


namespace vmime
{


class contentHandler;


/** Content encoding (basic type).
  */

class VMIME_EXPORT encoding : public headerFieldValue
{
public:

	enum EncodingUsage
	{
		USAGE_UNKNOWN,
		USAGE_TEXT,         /**< Use for body text. */
		USAGE_BINARY_DATA   /**< Use for attachment, image... */
	};


	encoding();
	explicit encoding(const string& name);
	encoding(const string& name, const EncodingUsage usage);
	encoding(const encoding& enc);

public:

	/** Return the name of the encoding.
	  * See the constants in vmime::encodingTypes.
	  *
	  * @return name of the encoding (eg. "quoted-printable")
	  */
	const string& getName() const;

	/** Set the name of the encoding.
	  * See the constants in vmime::encodingTypes.
	  *
	  * @param name name of the encoding
	  */
	void setName(const string& name);

	/** Return the type of contents this encoding is used for.
	  * See the EncodingUsage enum.
	  */
	EncodingUsage getUsage() const;

	/** Set the type of contents this encoding is used for.
	  * See the EncodingUsage enum.
	  *
	  * @param usage type of contents
	  */
	void setUsage(const EncodingUsage usage);


	encoding& operator=(const encoding& other);
	encoding& operator=(const string& name);

	bool operator==(const encoding& value) const;
	bool operator!=(const encoding& value) const;

	const std::vector <shared_ptr <component> > getChildComponents();

	/** Decide which encoding to use based on the specified data.
	  *
	  * @param data data used to determine encoding
	  * @param usage context of use of data
	  * @return suitable encoding for specified data
	  */
	static const encoding decide(shared_ptr <const contentHandler> data, const EncodingUsage usage = USAGE_BINARY_DATA);

	/** Decide which encoding to use based on the specified data and charset.
	  *
	  * @param data data used to determine encoding
	  * @param chset charset of data
	  * @param usage context of use of data
	  * @return suitable encoding for specified data and charset
	  */
	static const encoding decide(shared_ptr <const contentHandler> data, const charset& chset, const EncodingUsage usage = USAGE_BINARY_DATA);


	shared_ptr <component> clone() const;
	void copyFrom(const component& other);

	/** Use encoderFactory to obtain an encoder/decoder object
	  * for the current encoding type.
	  *
	  * @throw exceptions::no_encoder_available if no encoder
	  * is registered for the encoding
	  * @return a new encoder object for the encoding type
	  */
	shared_ptr <utility::encoder::encoder> getEncoder() const;

private:

	string m_name;
	EncodingUsage m_usage;

	/** Determine whether data encoded using this encoding should
	  * be re-encoded if needed.
	  *
	  * @return true if data should be re-encoded, false otherwise
	  */
	bool shouldReencode() const;

	/** Decide which encoding to use based on the specified data.
	  *
	  * Please note: this will read the whole buffer, so it should be used only
	  * for small amount of data (eg. text), and not large binary attachments.
	  *
	  * @param begin start iterator in buffer
	  * @param end end iterator in buffer
	  * @return suitable encoding for specified data
	  */
	static const encoding decideImpl(const string::const_iterator begin, const string::const_iterator end);

protected:

	// Component parsing & assembling
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
};


} // vmime


#endif // VMIME_ENCODING_HPP_INCLUDED
