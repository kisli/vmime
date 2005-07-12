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

#ifndef VMIME_MESSAGING_AUTHENTICATOR_HPP_INCLUDED
#define VMIME_MESSAGING_AUTHENTICATOR_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/messaging/authenticationInfos.hpp"


namespace vmime {
namespace messaging {


/** This class is used to obtain user credentials.
  */

class authenticator : public object
{
public:

	virtual ~authenticator();

	/** Called when the service needs to retrieve user credentials.
	  * It should return the user account name and password.
	  *
	  * @return user credentials (user name and password)
	  */
	virtual const authenticationInfos requestAuthInfos() const = 0;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_AUTHENTICATOR_HPP_INCLUDED
