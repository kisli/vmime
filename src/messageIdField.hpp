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

#ifndef VMIME_MESSAGEIDFIELD_HPP_INCLUDED
#define VMIME_MESSAGEIDFIELD_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"

#include "headerFieldFactory.hpp"
#include "messageId.hpp"


namespace vmime
{


class messageIdField : public headerField
{
	friend class headerFieldFactory::registerer <messageIdField>;

protected:

	messageIdField();

public:

	void copyFrom(const headerField& field);

	messageIdField& operator=(const messageId& mid);

	const messageId& value() const { return (m_id); }
	messageId& value() { return (m_id); }

protected:

	messageId m_id;

public:

	using headerField::parse;
	using headerField::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_MESSAGEIDFIELD_HPP_INCLUDED
