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

#include "path.hpp"

#include <algorithm>


namespace vmime {
namespace utility {


path::path()
{
}


path::path(const component& c)
{
	m_list.push_back(c);
}


path::path(const path& p)
{
	m_list.resize(p.m_list.size());
	std::copy(p.m_list.begin(), p.m_list.end(), m_list.begin());
}


path::path(const string& s)
{
	m_list.push_back(component(s));
}


path path::operator/(const path& p) const
{
	path pr(*this);
	pr /= p;

	return (pr);
}


path path::operator/(const component& c) const
{
	path pr(*this);
	pr /= c;

	return (pr);
}


path& path::operator/=(const path& p)
{
	const list::size_type size = m_list.size();

	m_list.resize(size + p.m_list.size());
	std::copy(p.m_list.begin(), p.m_list.end(), m_list.begin() + size);

	return (*this);
}


path& path::operator/=(const component& c)
{
	m_list.push_back(c);
	return (*this);
}


path path::parent() const
{
	path p;

	if (!empty())
	{
		p.m_list.resize(m_list.size() - 1);
		std::copy(m_list.begin(), m_list.end() - 1, p.m_list.begin());
	}

	return (p);
}


path& path::operator=(const path& p)
{
	m_list.resize(p.m_list.size());
	std::copy(p.m_list.begin(), p.m_list.end(), m_list.begin());

	return (*this);
}


path& path::operator=(const component& c)
{
	m_list.resize(1);
	m_list[0] = c;

	return (*this);
}


const bool path::operator==(const path& p) const
{
	if (m_list.size() != p.m_list.size())
		return (false);

	list::const_iterator i = m_list.begin();
	list::const_iterator j = p.m_list.begin();

	bool equal = true;

	for ( ; equal && i != m_list.end() ; ++i, ++j)
		//equal = (*i == *j);
		equal = ((*i).buffer() == (*j).buffer());

	return (equal);
}


const bool path::operator!=(const path& p) const
{
	return (!(*this == p));
}


const bool path::empty() const
{
	return (m_list.empty());
}


const path::component path::last() const
{
	return (empty() ? component("") : m_list[m_list.size() - 1]);
}


path::component& path::last()
{
	return (m_list[m_list.size() - 1]);
}


const int path::size() const
{
	return (m_list.size());
}


const path::component& path::operator[](const int x) const
{
	return (m_list[x]);
}


path::component& path::operator[](const int x)
{
	return (m_list[x]);
}


const bool path::isDirectParentOf(const path& p) const
{
	if (p.size() != size() + 1)
		return (false);

	bool equal = true;

	for (int i = 0 ; equal && i < size() ; ++i)
		equal = (m_list[i] == p.m_list[i]);

	return (equal);
}


} // utility
} // vmime
