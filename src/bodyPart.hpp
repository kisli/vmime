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

#ifndef VMIME_BODYPART_HPP_INCLUDED
#define VMIME_BODYPART_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"

#include "header.hpp"
#include "body.hpp"


namespace vmime
{


/** A MIME part.
  */

class bodyPart : public component
{
public:

	bodyPart();

	const class header& header() const { return (m_header); }
	class header& header() { return (m_header); }

	const class body& body() const { return (m_body); }
	class body& body() { return (m_body); }

	bodyPart* parent() const { return (m_parent); }

	bodyPart* clone() const;

protected:

	class header m_header;
	class body m_body;

	bodyPart* m_parent;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;


	// This is here because of a bug in g++ < 3.4
	friend class body;
	friend class body::partsContainer;
};


} // vmime


#endif // VMIME_BODYPART_HPP_INCLUDED
