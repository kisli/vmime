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

#include "textParameter.hpp"
#include "parserHelpers.hpp"


namespace vmime
{


void textParameter::parseValue(const string& buffer, const string::size_type position,
	const string::size_type end)
{
	m_value = string(buffer.begin() + position, buffer.begin() + end);
}


const string textParameter::generateValue() const
{
	return (m_value);
}


void textParameter::copyFrom(const parameter& param)
{
	const textParameter& source = dynamic_cast<const textParameter&>(param);
	m_value = source.m_value;

	defaultParameter::copyFrom(param);
}


} // vmime
