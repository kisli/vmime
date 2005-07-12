//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_ENCODING_HPP_INCLUDED
#define VMIME_ENCODING_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/encoder.hpp"


namespace vmime
{


class contentHandler;


/** Content encoding (basic type).
  */

class encoding : public component
{
public:

	encoding();
	encoding(const string& name);
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

	encoding& operator=(const encoding& other);
	encoding& operator=(const string& name);

	const bool operator==(const encoding& value) const;
	const bool operator!=(const encoding& value) const;

	const std::vector <ref <const component> > getChildComponents() const;

	/** Decide which encoding to use based on the specified data.
	  *
	  * \deprecated Use the new decide() method which takes a contentHandler parameter.
	  *
	  * @param begin start iterator in buffer
	  * @param end end iterator in buffer
	  * @return suitable encoding for specified data
	  */
	static const encoding decide(const string::const_iterator begin, const string::const_iterator end);

	/** Decide which encoding to use based on the specified data.
	  *
	  * @param data data used to determine encoding
	  * @return suitable encoding for specified data
	  */
	static const encoding decide(ref <const contentHandler> data);

	ref <component> clone() const;
	void copyFrom(const component& other);

	/** Use encoderFactory to obtain an encoder/decoder object
	  * for the current encoding type.
	  *
	  * @throw exceptions::no_encoder_available if no encoder
	  * is registered for the encoding
	  * @return a new encoder object for the encoding type
	  */
	ref <encoder> getEncoder() const;

private:

	string m_name;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_ENCODING_HPP_INCLUDED
