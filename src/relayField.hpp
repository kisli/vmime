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

#ifndef VMIME_RELAYFIELD_HPP_INCLUDED
#define VMIME_RELAYFIELD_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"

#include "headerFieldFactory.hpp"
#include "dateTime.hpp"


namespace vmime
{


class relayField : public headerField
{
	friend class headerFieldFactory::registerer <relayField>;

protected:

	relayField();

public:

	void copyFrom(const headerField& field);

	const string& from() const { return (m_from); }
	string& from() { return (m_from); }

	const string& via() const { return (m_via); }
	string& via() { return (m_via); }

	const string& by() const { return (m_by); }
	string& by() { return (m_by); }

	const string& id() const { return (m_id); }
	string& id() { return (m_id); }

	const string& for_() const { return (m_for); }
	string& for_() { return (m_for); }

	const datetime& date() const { return (m_date); }
	datetime& date() { return (m_date); }

	const std::vector <string>& with() const { return (m_with); }
	std::vector <string>& with() { return (m_with); }

protected:

	string m_from;
	string m_via;
	string m_by;
	string m_id;
	string m_for;
	std::vector <string> m_with;

	datetime m_date;

public:

	using headerField::parse;
	using headerField::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_RELAYFIELD_HPP_INCLUDED
