//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_UTILITY_PATH_HPP_INCLUDED
#define VMIME_UTILITY_PATH_HPP_INCLUDED


#include <vector>

#include "vmime/types.hpp"
#include "vmime/word.hpp"


namespace vmime {
namespace utility {


/** Abstract representation of a path (filesystem, mailbox, etc).
  */

class VMIME_EXPORT path : public object
{
public:

	typedef vmime::word component;
	typedef std::vector <component> list;

	// Construct a path
	path();
	path(const component& c);
	path(const path& p);
	explicit path(const string& s);

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
	bool operator==(const path& p) const;
	bool operator!=(const path& p) const;

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
	const component& getComponentAt(const size_t pos) const;

	/** Return the component at the specified position.
	  *
	  * @param pos position
	  * @return component at position 'pos'
	  */
	component& getComponentAt(const size_t pos);

	/** Test whether this path is empty (root).
	  *
	  * @return true if the path is empty (no components = root)
	  */
	bool isEmpty() const;

	/** Test whether this path is the root (alias for isEmpty()).
	  *
	  * @return true if the path is the root
	  */
	bool isRoot() const;

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
	size_t getSize() const;

	/** Return the specified component of the path (const version).
	  *
	  * @param x index of the component
	  * @return component at the specified index
	  */
	const component& operator[](const size_t x) const;

	/** Return the specified component of the path (non-const version).
	  *
	  * @param x index of the component
	  * @return component at the specified index
	  */
	component& operator[](const size_t x);

	/** Test whether this path is a direct parent of another one.
	  *
	  * @param p other path
	  * @return true if the specified path is a child
	  * of this path, false otherwise
	  */
	bool isDirectParentOf(const path& p) const;

	/** Test whether this path is a parent of another one.
	  *
	  * @param p other path
	  * @return true if the specified path is a child (direct or
	  * indirect) of this path, false otherwise
	  */
	bool isParentOf(const path& p) const;

	/** Rename a parent component in the path.
	  * Example: path("a/b/c/d").renameParent("a/b", "x/y/z")
	  * will return path("x/y/z/c/d").
	  *
	  * @param oldPath old parent path
	  * @param newPath new parent path
	  */
	void renameParent(const path& oldPath, const path& newPath);

	/** Construct a new path from a string.
	  *
	  * @param str string representation of the path
	  * @param sep separator string (eg: "/")
	  * @param cset charset in which the path is encoded (use the value returned by
	  * vmime::charset::getLocalCharset() to use the default charset of your system)
	  * @return a new path corresponding to the specified string
	  */
	static path fromString(const string& str, const string& sep, const charset& cset);

	/** Returns a string representation of this path.
	  *
	  * @param sep separator string (eg: "/")
	  * @param cset charset in which to encode the components (use the value returned by
	  * vmime::charset::getLocalCharset() to use the default charset of your system)
	  * @return a string representing this path
	  */
	const string toString(const string& sep, const charset& cset) const;

private:

	list m_list;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PATH_HPP_INCLUDED
