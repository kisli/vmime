//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/utility/path.hpp"

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
	: object()
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


path path::getParent() const
{
	path p;

	if (!isEmpty())
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


bool path::operator==(const path& p) const
{
	if (m_list.size() != p.m_list.size())
		return (false);

	list::const_iterator i = m_list.begin();
	list::const_iterator j = p.m_list.begin();

	bool equal = true;

	for ( ; equal && i != m_list.end() ; ++i, ++j)
		//equal = (*i == *j);
		equal = ((*i).getBuffer() == (*j).getBuffer());

	return (equal);
}


bool path::operator!=(const path& p) const
{
	return (!(*this == p));
}


bool path::isEmpty() const
{
	return (m_list.empty());
}


bool path::isRoot() const
{
	return (m_list.empty());
}


const path::component path::getLastComponent() const
{
	return (m_list[m_list.size() - 1]);
}


path::component& path::getLastComponent()
{
	return (m_list[m_list.size() - 1]);
}


int path::getSize() const
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


bool path::isDirectParentOf(const path& p) const
{
	if (p.getSize() != getSize() + 1)
		return (false);

	bool equal = true;

	for (list::size_type i = 0 ; equal && i < m_list.size() ; ++i)
		equal = (m_list[i] == p.m_list[i]);

	return (equal);
}


bool path::isParentOf(const path& p) const
{
	if (p.getSize() < getSize() + 1)
		return (false);

	bool equal = true;

	for (list::size_type i = 0 ; equal && i < m_list.size() ; ++i)
		equal = (m_list[i] == p.m_list[i]);

	return (equal);
}


void path::renameParent(const path& oldPath, const path& newPath)
{
	if (isEmpty() || oldPath.getSize() > getSize())
		return;

	bool equal = true;
	list::size_type i;

	for (i = 0 ; equal && i < oldPath.m_list.size() ; ++i)
		equal = (m_list[i] == oldPath.m_list[i]);

	if (i != oldPath.m_list.size())
		return;

	list newList;

	for (list::size_type j = 0 ; j < newPath.m_list.size() ; ++j)
		newList.push_back(newPath.m_list[j]);

	for (list::size_type j = i ; j < m_list.size() ; ++j)
		newList.push_back(m_list[j]);

	m_list.resize(newList.size());
	std::copy(newList.begin(), newList.end(), m_list.begin());
}


void path::appendComponent(const path::component& c)
{
	m_list.push_back(c);
}


const path::component& path::getComponentAt(const int pos) const
{
	return (m_list[pos]);
}


path::component& path::getComponentAt(const int pos)
{
	return (m_list[pos]);
}


} // utility
} // vmime
