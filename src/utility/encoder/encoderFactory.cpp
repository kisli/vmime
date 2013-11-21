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

	// Finally, register some bogus encoding names, for compatibility
	registerName <qpEncoder>("bmoted-printable");
}


encoderFactory::~encoderFactory()
{
}


shared_ptr <encoderFactory> encoderFactory::getInstance()
{
	static encoderFactory instance;
	return shared_ptr <encoderFactory>(&instance, noop_shared_ptr_deleter <encoderFactory>());
}


shared_ptr <encoder> encoderFactory::create(const string& name)
{
	return (getEncoderByName(name)->create());
}


const shared_ptr <const encoderFactory::registeredEncoder> encoderFactory::getEncoderByName(const string& name) const
{
	const string lcName(utility::stringUtils::toLower(name));

	for (std::vector <shared_ptr <registeredEncoder> >::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		if ((*it)->getName() == lcName)
			return (*it);
	}

	throw exceptions::no_encoder_available(name);
}


size_t encoderFactory::getEncoderCount() const
{
	return (m_encoders.size());
}


const shared_ptr <const encoderFactory::registeredEncoder> encoderFactory::getEncoderAt(const size_t pos) const
{
	return (m_encoders[pos]);
}


const std::vector <shared_ptr <const encoderFactory::registeredEncoder> > encoderFactory::getEncoderList() const
{
	std::vector <shared_ptr <const registeredEncoder> > res;

	for (std::vector <shared_ptr <registeredEncoder> >::const_iterator it = m_encoders.begin() ;
	     it != m_encoders.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


} // encoder
} // utility
} // vmime
