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

#ifndef VMIME_DISPOSITION_HPP_INCLUDED
#define VMIME_DISPOSITION_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Content disposition (basic type).
  */

class disposition : public component
{
public:

	disposition();
	disposition(const string& name);
	disposition(const disposition& disp);

public:

	const string& name() const { return (m_name); }
	string& name() { return (m_name); }

public:

	disposition& operator=(const disposition& source);
	disposition& operator=(const string& name);

	const bool operator==(const disposition& value) const;
	const bool operator!=(const disposition& value) const;

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


#endif // VMIME_DISPOSITION_HPP_INCLUDED
