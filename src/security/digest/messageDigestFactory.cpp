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

#include "vmime/security/digest/messageDigestFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/security/digest/md5/md5MessageDigest.hpp"
#include "vmime/security/digest/sha1/sha1MessageDigest.hpp"


namespace vmime {
namespace security {
namespace digest {


messageDigestFactory::messageDigestFactory()
{
	registerAlgorithm <md5::md5MessageDigest>("md5");
	registerAlgorithm <sha1::sha1MessageDigest>("sha1");
}


messageDigestFactory::~messageDigestFactory()
{
}


messageDigestFactory* messageDigestFactory::getInstance()
{
	static messageDigestFactory instance;
	return (&instance);
}


ref <messageDigest> messageDigestFactory::create(const string& name)
{
	const MapType::const_iterator it = m_algos.find
		(utility::stringUtils::toLower(name));

	if (it != m_algos.end())
		return (*it).second->create();

	throw exceptions::no_digest_algorithm_available(name);
}


const std::vector <string> messageDigestFactory::getSupportedAlgorithms() const
{
	std::vector <string> res;

	for (MapType::const_iterator it = m_algos.begin() ;
	     it != m_algos.end() ; ++it)
	{
		res.push_back((*it).first);
	}

	return res;
}


} // digest
} // security
} // vmime

