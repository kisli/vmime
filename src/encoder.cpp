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

#include "encoder.hpp"
#include "exception.hpp"


namespace vmime
{


encoder::encoder()
{
}


encoder::~encoder()
{
}


const propertySet& encoder::properties() const
{
	return (m_props);
}


propertySet& encoder::properties()
{
	return (m_props);
}


const propertySet& encoder::results() const
{
	return (m_results);
}


propertySet& encoder::results()
{
	return (m_results);
}


const std::vector <string> encoder::availableProperties() const
{
	std::vector <string> list;
	return (list);
}


} // vmime
