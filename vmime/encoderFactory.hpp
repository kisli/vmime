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

#ifndef VMIME_ENCODERFACTORY_HPP_INCLUDED
#define VMIME_ENCODERFACTORY_HPP_INCLUDED


#include "vmime/encoder.hpp"
#include "vmime/utility/singleton.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


/** A factory to create 'encoder' objects for the specified encoding.
  */

class encoderFactory : public utility::singleton <encoderFactory>
{
	friend class utility::singleton <encoderFactory>;

private:

	encoderFactory();
	~encoderFactory();

public:

	/** Information about a registered encoder. */
	class registeredEncoder
	{
		friend class encoderFactory;

	protected:

		virtual ~registeredEncoder() { }

	public:

		virtual encoder* create() const = 0;

		virtual const string& getName() const = 0;
	};

private:

	template <class E>
	class registeredEncoderImpl : public registeredEncoder
	{
		friend class encoderFactory;

	protected:

		registeredEncoderImpl(const string& name) : m_name(name) { }

	public:

		encoder* create() const
		{
			return new E;
		}

		const string& getName() const
		{
			return (m_name);
		}

	private:

		const string m_name;
	};


	std::vector <registeredEncoder*> m_encoders;

public:

	/** Register a new encoder by its encoding name.
	  *
	  * @param name encoding name
	  */
	template <class E>
	void registerName(const string& name)
	{
		m_encoders.push_back(new registeredEncoderImpl <E>(stringUtils::toLower(name)));
	}

	/** Create a new encoder instance from an encoding name.
	  *
	  * @param name encoding name (eg. "base64")
	  * @return a new encoder instance for the specified encoding
	  * @throw exceptions::no_encoder_available if no encoder is registered
	  * for this encoding
	  */
	encoder* create(const string& name);

	/** Return information about a registered encoder.
	  *
	  * @param name encoding name
	  * @return information about this encoder
	  * @throw exceptions::no_encoder_available if no encoder is registered
	  * for this encoding
	  */
	const registeredEncoder* getEncoderByName(const string& name) const;

	/** Return the number of registered encoders.
	  *
	  * @return number of registered encoders
	  */
	const int getEncoderCount() const;

	/** Return the registered encoder at the specified position.
	  *
	  * @param pos position of the registered encoder to return
	  * @return registered encoder at the specified position
	  */
	const registeredEncoder* getEncoderAt(const int pos) const;

	/** Return a list of all registered encoders.
	  *
	  * @return list of registered encoders
	  */
	const std::vector <const registeredEncoder*> getEncoderList() const;
};


} // vmime


#endif // VMIME_ENCODERFACTORY_HPP_INCLUDED
