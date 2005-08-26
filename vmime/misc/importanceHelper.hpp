//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MISC_IMPORTANCEHELPER_HPP_INCLUDED
#define VMIME_MISC_IMPORTANCEHELPER_HPP_INCLUDED


#include "vmime/message.hpp"


namespace vmime {
namespace misc {


/** Deals with setting and retrieving message importance (also
  * known as priority).
  *
  * Basically, it wraps the use of the 'X-Priority' (non standard)
  * and 'Importance' (RFC-1327, RFC-1911) fields.
  */

class importanceHelper
{
public:

	/** Different levels of importance. */
	enum Importance
	{
		IMPORTANCE_HIGHEST,
		IMPORTANCE_HIGH,
		IMPORTANCE_NORMAL,
		IMPORTANCE_LOW,
		IMPORTANCE_LOWEST
	};


	/** Reset the importance of the message to the default importance.
	  *
	  * @param msg message on which to reset importance
	  */
	static void resetImportance(ref <message> msg);

	/** Reset the importance of a message to the default importance.
	  *
	  * @param hdr message header on which to reset importance
	  */
	static void resetImportanceHeader(ref <header> hdr);

	/** Return the importance of the specified message.
	  *
	  * @param msg message from which to retrieve importance
	  * @return importance of the message, or default importance is no
	  * information about importance is given in the message
	  */
	static const Importance getImportance(ref <const message> msg);

	/** Return the importance of a message, given its header.
	  *
	  * @param hdr message header from which to retrieve importance
	  * @return importance of the message, or default importance is no
	  * information about importance is given in the message
	  */
	static const Importance getImportanceHeader(ref <const header> hdr);

	/** Set the importance of the specified message.
	  *
	  * @param msg message on which to set importance
	  * @param i new message importance
	  */
	static void setImportance(ref <message> msg, const Importance i);

	/** Set the importance of a message, given its header.
	  *
	  * @param hdr message header on which to set importance
	  * @param i new message importance
	  */
	static void setImportanceHeader(ref <header> hdr, const Importance i);
};


} // misc
} // vmime


#endif // VMIME_MISC_IMPORTANCEHELPER_HPP_INCLUDED
