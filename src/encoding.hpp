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

#ifndef VMIME_ENCODING_HPP_INCLUDED
#define VMIME_ENCODING_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"
#include "encoder.hpp"


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

	const string& name() const { return (m_name); }
	string& name() { return (m_name); }

public:

	encoding& operator=(const encoding& source);
	encoding& operator=(const string& name);

	const bool operator==(const encoding& value) const;
	const bool operator!=(const encoding& value) const;

	// Decide which encoding to use based on the data
	static const encoding decide(const string::const_iterator begin, const string::const_iterator end);
	static const encoding decide(const contentHandler& data);

public:

	// Obtain an encoder/decoder for the current encoding type
	encoder* getEncoder() const;

protected:

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
