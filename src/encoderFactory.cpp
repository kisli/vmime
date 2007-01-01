//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2007 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/encoderFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/encoderB64.hpp"
#include "vmime/encoderQP.hpp"
#include "vmime/encoderUUE.hpp"
#include "vmime/encoderBinary.hpp"
#include "vmime/encoder7bit.hpp"
#include "vmime/encoder8bit.hpp"


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
}


encoderFactory* encoderFactory::getInstance()
{
	static encoderFactory instance;
	return (&instance);
}


ref <encoder> encoderFactory::create(const string& name)
{
	return (getEncoderByName(name)->create());
}


const ref <const encoderFactory::registeredEncoder> encoderFactory::getEncoderByName(const string& name) const
{
	const string lcName(utility::stringUtils::toLower(name));

	for (std::vector <ref <registeredEncoder> >::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		if ((*it)->getName() == lcName)
			return (*it);
	}

	throw exceptions::no_encoder_available(name);
}


const int encoderFactory::getEncoderCount() const
{
	return (m_encoders.size());
}


const ref <const encoderFactory::registeredEncoder> encoderFactory::getEncoderAt(const int pos) const
{
	return (m_encoders[pos]);
}


const std::vector <ref <const encoderFactory::registeredEncoder> > encoderFactory::getEncoderList() const
{
	std::vector <ref <const registeredEncoder> > res;

	for (std::vector <ref <registeredEncoder> >::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


} // vmime
