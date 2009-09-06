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

#ifndef VMIME_NET_SERVICEINFOS_HPP_INCLUDED
#define VMIME_NET_SERVICEINFOS_HPP_INCLUDED


#include <vector>

#include "vmime/types.hpp"

#include "vmime/net/session.hpp"


namespace vmime {
namespace net {


/** Stores information about a messaging service.
  */

class serviceInfos
{
	friend class serviceFactory;

protected:

	serviceInfos();
	serviceInfos(const serviceInfos&);

private:

	serviceInfos& operator=(const serviceInfos&);

public:

	virtual ~serviceInfos();


	/** A service property.
	  */
	class property
	{
	public:

		/** The common property 'server.address' which is
		  * the host name or the IP address of the server. */
		static const property SERVER_ADDRESS;

		/** The common property 'server.port' which is
		  * the port used to connect to the server. */
		static const property SERVER_PORT;

		/** The common property 'server.rootpath' which is
		  * the full path of the folder on the server (for
		  * maildir, this is the local filesystem directory). */
		static const property SERVER_ROOTPATH;

		/** The common property 'auth.username' which is the
		  * username used to authenticate with the server. */
		static const property AUTH_USERNAME;

		/** The common property 'auth.password' which is the
		  * password used to authenticate with the server. */
		static const property AUTH_PASSWORD;

#if VMIME_HAVE_TLS_SUPPORT

		/** The common property 'connection.tls': this is used to
		  * start a secured connection if it is supported by the
		  * server (STARTTLS extension).
		  */
		static const property CONNECTION_TLS;

		/** The common property 'connection.tls.required' should be
		  * set to 'true' to make the connection process fail if the
		  * server can't start a secured connection (no effect if
		  * 'connection.tls' is not set to 'true').
		  */
		static const property CONNECTION_TLS_REQUIRED;

#endif // VMIME_HAVE_TLS_SUPPORT


		/** Value types.
		  */
		enum Types
		{
			TYPE_INTEGER,   /*< Integer number. */
			TYPE_STRING,    /*< Character string. */
			TYPE_BOOL,      /*< Boolean (true or false). */

			TYPE_DEFAULT = TYPE_STRING
		};

		/** Property flags.
		  */
		enum Flags
		{
			FLAG_NONE = 0,              /*< No flags. */
			FLAG_REQUIRED = (1 << 0),   /*< The property must be valued. */
			FLAG_HIDDEN = (1 << 1),     /*< The property should not be shown
			                                to the user but can be modified. */

			FLAG_DEFAULT = FLAG_NONE    /*< Default flags. */
		};


		/** Construct a new property.
		  *
		  * @param name property name
		  * @param type value type
		  * @param defaultValue default value
		  * @param flags property attributes
		  */
		property(const string& name, const Types type, const string& defaultValue = "", const int flags = FLAG_DEFAULT);

		/** Construct a new property from an existing property.
		  *
		  * @param p source property
		  * @param addFlags flags to add
		  * @param removeFlags flags to remove
		  */
		property(const property& p, const int addFlags = FLAG_NONE, const int removeFlags = FLAG_NONE);

		/** Construct a new property from an existing property.
		  *
		  * @param p source property
		  * @param newDefaultValue new default value
		  * @param addFlags flags to add
		  * @param removeFlags flags to remove
		  */
		property(const property& p, const string& newDefaultValue, const int addFlags = FLAG_NONE, const int removeFlags = FLAG_NONE);

		property& operator=(const property& p);

		/** Return the name of the property.
		  *
		  * @return property name
		  */
		const string& getName() const;

		/** Return the default value of the property or
		  * an empty string if there is no default value.
		  *
		  * @return default value for the property
		  */
		const string& getDefaultValue() const;

		/** Return the value type of the property.
		  *
		  * @return property value type
		  */
		Types getType() const;

		/** Return the attributes of the property (see
		  * serviceInfos::property::Types constants).
		  *
		  * @return property attributes
		  */
		int getFlags() const;

	private:

		string m_name;
		string m_defaultValue;
		Types m_type;
		int m_flags;
	};


	/** Return the property prefix used by this service.
	  * Use this to set/get properties in the session object.
	  *
	  * @return property prefix
	  */
	virtual const string getPropertyPrefix() const = 0;

	/** Return a list of available properties for this service.
	  *
	  * @return list of properties
	  */
	virtual const std::vector <property> getAvailableProperties() const = 0;

	/** Helper function to retrieve the value of a property.
	  *
	  * @param s session object
	  * @param p property to retrieve
	  * @throw exceptions::no_such_property if the property does not exist
	  * and has the flag property::FLAG_REQUIRED
	  * @return value of the property
	  */
	template <typename TYPE>
	const TYPE getPropertyValue(ref <session> s, const property& p) const
	{
		if (p.getFlags() & property::FLAG_REQUIRED)
			return s->getProperties()[getPropertyPrefix() + p.getName()].template getValue <TYPE>();

		return s->getProperties().template getProperty <TYPE>(getPropertyPrefix() + p.getName(),
			propertySet::valueFromString <TYPE>(p.getDefaultValue()));
	}

	/** Helper function to test if the specified property is set in
	  * the session object.
	  *
	  * @param s session object
	  * @param p property to test
	  * @return true if the property is set, false otherwise
	  */
	bool hasProperty(ref <session> s, const property& p) const;
};


} // net
} // vmime


#endif // VMIME_NET_SERVICEINFOS_HPP_INCLUDED
