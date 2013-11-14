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

#ifndef VMIME_NET_FETCHATTRIBUTES_HPP_INCLUDED
#define VMIME_NET_FETCHATTRIBUTES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include <vector>

#include "vmime/types.hpp"


namespace vmime {
namespace net {


/** Holds a set of attributes to fetch for a message.
  */
class VMIME_EXPORT fetchAttributes : public object
{
public:

	/** Predefined attributes that can be fetched.
	  */
	enum PredefinedFetchAttributes
	{
		ENVELOPE = (1 << 0),       /**< Sender, recipients, date, subject. */
		STRUCTURE = (1 << 1),      /**< MIME structure (body parts). */
		CONTENT_INFO = (1 << 2),   /**< Top-level content type. */
		FLAGS = (1 << 3),          /**< Message flags. */
		SIZE = (1 << 4),           /**< Message size (exact or estimated). */
		FULL_HEADER = (1 << 5),    /**< Full RFC-[2]822 header. */
		UID = (1 << 6),            /**< Unique identifier (protocol specific). */
		IMPORTANCE = (1 << 7),     /**< Header fields suitable for use with misc::importanceHelper. */

		CUSTOM = (1 << 16)         /**< Reserved for future use. */
	};

	/** Constructs an empty fetchAttributes object.
	  */
	fetchAttributes();

	/** Constructs a new fetchAttributes object by specifying one or more
	  * predefined objects.
	  *
	  * @param attribs one or more OR-ed values of the PredefinedFetchAttributes enum
	  */
	fetchAttributes(const int attribs);

	/** Constructs a new fetchAttributes object by copying an existing object.
	  *
	  * @param attribs object to copy
	  */
	fetchAttributes(const fetchAttributes& attribs);

	/** Adds the specified predefined attribute to the set of attributes to fetch.
	  *
	  * @param attribs one or more OR-ed values of the PredefinedFetchAttributes enum
	  */
	void add(const int attribs);

	/** Adds the specified header field to the set of attributes to fetch.
	  * Fetching custom header fields is not supported by all protocols.
	  * At this time, only IMAP supports this.
	  *
	  * @param header name of header field (eg. "X-Mailer")
	  */
	void add(const string& header);

	/** Returns true if the set contains the specified attribute(s).
	  *
	  * @param attribs one or more OR-ed values of the PredefinedFetchAttributes enum
	  * @return true if the specified attributes are to be fetched
	  */
	bool has(const int attribs) const;

	/** Returns true if the set contains the specified header field.
	  *
	  * @param header name of header field (eg. "X-Mailer")
	  * @return true if the specified header fields are to be fetched
	  */
	bool has(const string& header) const;

	/** Returns true if the set contains the specified attribute(s).
	  *
	  * \deprecated Use the has() methods instead
	  *
	  * @param attribs one or more OR-ed values of the PredefinedFetchAttributes enum
	  * @return true if the specified attributes are to be fetched
	  */
	VMIME_DEPRECATED inline bool operator&(const int attribs) const
	{
		return has(attribs);
	}

	/** Returns a list of header fields to fetch.
	  *
	  * @return list of header names (eg. "X-Mailer")
	  */
	const std::vector <string> getHeaderFields() const;

private:

	int m_predefinedAttribs;
	std::vector <string> m_headers;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_NET_FETCHATTRIBUTES_HPP_INCLUDED
