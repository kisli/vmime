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

#include "component.hpp"
#include "base.hpp"

#include <sstream>


namespace vmime
{


component::component()
	: m_parsedOffset(0), m_parsedLength(0)
{
}


component::~component()
{
}


void component::parse(const string& buffer)
{
	parse(buffer, 0, buffer.length(), NULL);
}


const string component::generate(const string::size_type maxLineLength,
	const string::size_type curLinePos) const
{
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	generate(adapter, maxLineLength, curLinePos, NULL);

	return (oss.str());
}


const string::size_type component::getParsedOffset() const
{
	return (m_parsedOffset);
}


const string::size_type component::getParsedLength() const
{
	return (m_parsedLength);
}


void component::setParsedBounds(const string::size_type start, const string::size_type end)
{
	m_parsedOffset = start;
	m_parsedLength = end - start;
}


const std::vector <component*> component::getChildComponents()
{
	const std::vector <const component*> constList =
		const_cast <const component*>(this)->getChildComponents();

	std::vector <component*> list;

	const std::vector <const component*>::size_type count = constList.size();

	list.resize(count);

	for (std::vector <const component*>::size_type i = 0 ; i < count ; ++i)
		list[i] = const_cast <component*>(constList[i]);

	return (list);
}


}
