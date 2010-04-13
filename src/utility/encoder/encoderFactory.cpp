//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/encoder/encoderFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/utility/encoder/b64Encoder.hpp"
#include "vmime/utility/encoder/qpEncoder.hpp"
#include "vmime/utility/encoder/uuEncoder.hpp"
#include "vmime/utility/encoder/binaryEncoder.hpp"
#include "vmime/utility/encoder/sevenBitEncoder.hpp"
#include "vmime/utility/encoder/eightBitEncoder.hpp"


namespace vmime {
namespace utility {
namespace encoder {


encoderFactory::encoderFactory()
{
	// Register some default encoders
	registerName <b64Encoder>("base64");
	registerName <qpEncoder>("quoted-printable");
	registerName <uuEncoder>("uuencode");
	registerName <sevenBitEncoder>("7bit");
	registerName <eightBitEncoder>("8bit");
	registerName <binaryEncoder>("binary");

	// Also register some non-standard encoding names
	registerName <sevenBitEncoder>("7-bit");
	registerName <eightBitEncoder>("8-bit");
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


int encoderFactory::getEncoderCount() const
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


} // encoder
} // utility
} // vmime
