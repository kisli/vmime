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

#ifndef VMIME_UTILITY_PATH_HPP_INCLUDED
#define VMIME_UTILITY_PATH_HPP_INCLUDED


#include <vector>

#include "../types.hpp"
#include "../word.hpp"


namespace vmime {
namespace utility {


/** Abstract representation of a path (filesystem, mailbox, etc).
  */

class path
{
public:

	typedef vmime::word component;
	typedef std::vector <component> list;

	// Construct a path
	path();
	path(const component& c);
	path(const path& p);
	path(const string& s);

	// Append a component to a path
	path operator/(const path& p) const;
	path operator/(const component& c) const;

	path& operator/=(const path& p);
	path& operator/=(const component& c);

	// Return the parent path
	path getParent() const;

	// Assignment
	path& operator=(const path& p);
	path& operator=(const component& c);

	// Path comparison
	const bool operator==(const path& p) const;
	const bool operator!=(const path& p) const;

	/** Append a component to the path.
	  *
	  * @param c component to add
	  */
	void appendComponent(const component& c);

	/** Return the component at the specified position.
	  *
	  * @param pos position
	  * @return component at position 'pos'
	  */
	const component& getComponentAt(const int pos) const;

	/** Return the component at the specified position.
	  *
	  * @param pos position
	  * @return component at position 'pos'
	  */
	component& getComponentAt(const int pos);

	/** Test whether this path is empty (root).
	  *
	  * @return true if the path is empty (no components = root)
	  */
	const bool isEmpty() const;

	/** Return the last component of this path (const version).
	  *
	  * @return last component
	  */
	const component getLastComponent() const;

	/** Return the last component of this path (non-const version).
	  *
	  * @return last component
	  */
	component& getLastComponent();

	/** Return the number of components in this path.
	  *
	  * @return number of components
	  */
	const int getSize() const;

	/** Return the specified component of the path (const version).
	  *
	  * @param x index of the component
	  * @return component at the specified index
	  */
	const component& operator[](const int x) const;

	/** Return the specified component of the path (non-const version).
	  *
	  * @param x index of the component
	  * @return component at the specified index
	  */
	component& operator[](const int x);

	/** Test whether this path is a direct parent of another one.
	  *
	  * @param p other path
	  * @return true if the specified path is a child
	  * of this path, false otherwise
	  */
	const bool isDirectParentOf(const path& p) const;

	/** Test whether this path is a parent of another one.
	  *
	  * @param p other path
	  * @return true if the specified path is a child (direct or
	  * indirect) of this path, false otherwise
	  */
	const bool isParentOf(const path& p) const;

	/** Rename a parent component in the path.
	  *
	  * @param oldPath old parent path
	  * @param newPath new parent path
	  */
	void renameParent(const path& oldPath, const path& newPath);

private:

	list m_list;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PATH_HPP_INCLUDED
