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

#include "encoderFactory.hpp"
#include "exception.hpp"

#include "encoderB64.hpp"
#include "encoderQP.hpp"
#include "encoderUUE.hpp"
#include "encoderBinary.hpp"
#include "encoder7bit.hpp"
#include "encoder8bit.hpp"


namespace vmime
{


encoderFactory::encoderFactory()
{
	// Register some default encoders
	registerName <encoderB64>("base64");
	registerName <encoderQP>("quoted-printable");
	registerName <encoderUUE>("uuencode");
	registerName <encoder7bit>("7bit");
	registerName <encoder8bit>("8bit");
	registerName <encoderBinary>("binary");
}


encoderFactory::~encoderFactory()
{
	for (NameMap::iterator it = m_nameMap.begin() ; it != m_nameMap.end() ; ++it)
		delete ((*it).second);
}


encoder* encoderFactory::create(const string& name)
{
	NameMap::const_iterator pos = m_nameMap.find(toLower(name));

	if (pos != m_nameMap.end())
	{
		return ((*pos).second)->create();
	}
	else
	{
		throw exceptions::no_encoder_available();
		return (NULL);
	}
}


} // vmime
