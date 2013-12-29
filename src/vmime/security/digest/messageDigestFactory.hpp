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

#ifndef VMIME_SECURITY_DIGEST_MESSAGEDIGESTFACTORY_HPP_INCLUDED
#define VMIME_SECURITY_DIGEST_MESSAGEDIGESTFACTORY_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/security/digest/messageDigest.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime {
namespace security {
namespace digest {


/** Creates instances of message digest algorithms.
  */

class VMIME_EXPORT messageDigestFactory
{
private:

	messageDigestFactory();
	~messageDigestFactory();

public:

	static messageDigestFactory* getInstance();

private:

	class digestAlgorithmFactory : public object
	{
	public:

		virtual shared_ptr <messageDigest> create() const = 0;
	};

	template <class E>
	class digestAlgorithmFactoryImpl : public digestAlgorithmFactory
	{
	public:

		shared_ptr <messageDigest> create() const
		{
			return vmime::make_shared <E>();
		}
	};


	typedef std::map <string, shared_ptr <digestAlgorithmFactory> > MapType;
	MapType m_algos;

public:

	/** Register a new digest algorithm by its name.
	  *
	  * @param name algorithm name
	  */
	template <class E>
	void registerAlgorithm(const string& name)
	{
		m_algos.insert(MapType::value_type(utility::stringUtils::toLower(name),
			vmime::make_shared <digestAlgorithmFactoryImpl <E> >()));
	}

	/** Create a new algorithm instance from its name.
	  *
	  * @param name algorithm name (eg. "md5")
	  * @return a new algorithm instance for the specified name
	  * @throw exceptions::no_digest_algorithm_available if no algorithm is
	  * registered with this name
	  */
	shared_ptr <messageDigest> create(const string& name);

	/** Return a list of supported digest algorithms.
	  *
	  * @return list of supported digest algorithms
	  */
	const std::vector <string> getSupportedAlgorithms() const;
};


} // digest
} // security
} // vmime


#endif // VMIME_SECURITY_DIGEST_MESSAGEDIGESTFACTORY_HPP_INCLUDED

