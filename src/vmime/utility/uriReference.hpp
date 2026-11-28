//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_UTILITY_URIREFERENCE_HPP_INCLUDED
#define VMIME_UTILITY_URIREFERENCE_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/base.hpp"


namespace vmime {
namespace utility {


/** A URI reference (RFC 3986, section 4.1): an absolute URI, or a
  * relative reference.
  *
  * Unlike url, this class is purely syntactic: any string is accepted,
  * and percent-encoded octets are not decoded, so that a reference is
  * converted back to the string it was parsed from.
  */
class VMIME_EXPORT uriReference {

public:

	/** Construct an empty reference.
	  */
	uriReference();

	/** Construct a reference by splitting a string into its components,
	  * as the regular expression of RFC 3986, appendix B.
	  *
	  * @param ref URI reference
	  */
	explicit uriReference(const string& ref);

	/** Return whether the reference has a scheme, ie. whether it is
	  * an absolute URI rather than a relative reference.
	  *
	  * @return true if the reference has a scheme, false otherwise
	  */
	bool hasScheme() const;

	/** Return the scheme of the reference.
	  *
	  * @return scheme, or an empty string if there is none
	  */
	const string& getScheme() const;

	/** Return whether the reference has an authority ("//" followed by
	  * the host name, for example).
	  *
	  * @return true if the reference has an authority, false otherwise
	  */
	bool hasAuthority() const;

	/** Return the authority of the reference.
	  *
	  * @return authority, or an empty string if there is none
	  */
	const string& getAuthority() const;

	/** Return the path of the reference, which can be empty.
	  *
	  * @return path
	  */
	const string& getPath() const;

	/** Return whether the reference has a query (after a '?').
	  *
	  * @return true if the reference has a query, false otherwise
	  */
	bool hasQuery() const;

	/** Return the query of the reference.
	  *
	  * @return query, or an empty string if there is none
	  */
	const string& getQuery() const;

	/** Return whether the reference has a fragment (after a '#').
	  *
	  * @return true if the reference has a fragment, false otherwise
	  */
	bool hasFragment() const;

	/** Return the fragment of the reference.
	  *
	  * @return fragment, or an empty string if there is none
	  */
	const string& getFragment() const;

	/** Resolve the reference against a base URI, as specified in
	  * RFC 3986, section 5.2 (strict parser).
	  *
	  * @param base base URI, which should be absolute
	  * @return target URI
	  */
	uriReference resolve(const uriReference& base) const;

	/** Recompose the reference (RFC 3986, section 5.3).
	  *
	  * @return reference as a string
	  */
	string toString() const;

private:

	// The "has" members tell an undefined component from an empty one
	bool m_hasScheme;
	bool m_hasAuthority;
	bool m_hasQuery;
	bool m_hasFragment;

	string m_scheme;
	string m_authority;
	string m_path;
	string m_query;
	string m_fragment;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_URIREFERENCE_HPP_INCLUDED
