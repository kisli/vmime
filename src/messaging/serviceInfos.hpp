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

#ifndef VMIME_MESSAGING_SERVICEINFOS_HPP_INCLUDED
#define VMIME_MESSAGING_SERVICEINFOS_HPP_INCLUDED


#include <vector>

#include "../types.hpp"


namespace vmime {
namespace messaging {


/** Stores information about a messaging service.
  */

class serviceInfos
{
	friend class serviceFactory;

protected:

	serviceInfos() { }
	serviceInfos(const serviceInfos&) { }

private:

	serviceInfos& operator=(const serviceInfos&) { return (*this); }

public:

	virtual ~serviceInfos() { }

	/** Return the default port used for the underlying protocol.
	  *
	  * @return default port number
	  */
	virtual const port_t getDefaultPort() const = 0;

	/** Return the property prefix used by this service.
	  * Use this to set/get properties in the session object.
	  *
	  * @return property prefix
	  */
	virtual const string getPropertyPrefix() const = 0;

	/** Return a list of available properties for this service.
	  *
	  * @return list of property names
	  */
	virtual const std::vector <string> getAvailableProperties() const = 0;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_SERVICEINFOS_HPP_INCLUDED
