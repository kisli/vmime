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

#ifndef VMIME_TEXTPARAMETER_HPP_INCLUDED
#define VMIME_TEXTPARAMETER_HPP_INCLUDED


#include "defaultParameter.hpp"


namespace vmime
{


class textParameter : public defaultParameter
{
protected:

	string m_value;

public:

	void copyFrom(const parameter& param);

	const string& value() const { return (m_value); }
	string& value() { return (m_value); }

protected:

	void parseValue(const string& buffer, const string::size_type position, const string::size_type end);
	const string generateValue() const;
};


} // vmime


#endif // VMIME_TEXTPARAMETER_HPP_INCLUDED
