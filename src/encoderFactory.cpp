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
	for (std::vector <registeredEncoder*>::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		delete (*it);
	}
}


encoder* encoderFactory::create(const string& name)
{
	return (getEncoderByName(name)->create());
}


const encoderFactory::registeredEncoder* encoderFactory::getEncoderByName(const string& name) const
{
	const string lcName(stringUtils::toLower(name));

	for (std::vector <registeredEncoder*>::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		if ((*it)->getName() == lcName)
			return (*it);
	}

	throw exceptions::no_encoder_available();
}


const int encoderFactory::getEncoderCount() const
{
	return (m_encoders.size());
}


const encoderFactory::registeredEncoder* encoderFactory::getEncoderAt(const int pos) const
{
	return (m_encoders[pos]);
}


const std::vector <const encoderFactory::registeredEncoder*> encoderFactory::getEncoderList() const
{
	std::vector <const registeredEncoder*> res;

	for (std::vector <registeredEncoder*>::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


} // vmime
