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

#ifndef VMIME_MESSAGING_SERVICEFACTORY_HPP_INCLUDED
#define VMIME_MESSAGING_SERVICEFACTORY_HPP_INCLUDED


#include <map>

#include "../types.hpp"
#include "../base.hpp"
#include "../utility/singleton.hpp"

#include "serviceInfos.hpp"
#include "authenticator.hpp"
#include "progressionListener.hpp"
#include "timeoutHandler.hpp"
#include "url.hpp"


namespace vmime {
namespace messaging {


class service;
class session;


/** A factory to create 'service' objects for a specified protocol.
  */

class serviceFactory : public utility::singleton <serviceFactory>
{
	friend class utility::singleton <serviceFactory>;

protected:

	serviceFactory();
	~serviceFactory();

public:

	class registeredService
	{
		friend class serviceFactory;

	protected:

		virtual ~registeredService() { }

	public:

		virtual service* create(session& sess, authenticator* auth) = 0;

		virtual const string& name() const = 0;
		virtual const serviceInfos& infos() const = 0;
	};

private:

	template <class S>
	class registeredServiceImpl : public registeredService
	{
		friend class serviceFactory;

	protected:

		registeredServiceImpl(const string& name)
			: m_name(name), m_servInfos(S::infosInstance())
		{
		}

	public:

		service* create(session& sess, authenticator* auth)
		{
			return new S(sess, auth);
		}

		const serviceInfos& infos() const
		{
			return (m_servInfos);
		}

		const string& name() const
		{
			return (m_name);
		}

	private:

		const string m_name;
		const serviceInfos& m_servInfos;
	};

	typedef std::map <string, registeredService*> ProtoMap;
	ProtoMap m_protoMap;

public:

	template <class S>
	void registerName(const string& protocol)
	{
		const string name = vmime::toLower(protocol);
		m_protoMap.insert(ProtoMap::value_type(name,
			new registeredServiceImpl <S>(name)));
	}

	service* create(session& sess, const string& protocol, authenticator* auth = NULL);
	service* create(session& sess, const url& u, authenticator* auth = NULL);

	const registeredService& operator[](const string& protocol) const;


	class iterator;

	class const_iterator
	{
		friend class serviceFactory;

	public:

		const_iterator() { }
		const_iterator(const const_iterator& it) : m_it(it.m_it) { }
		const_iterator(const iterator& it) : m_it(it.m_it) { }

		const_iterator& operator=(const const_iterator& it) { m_it = it.m_it; return (*this); }

		const registeredService& operator*() const { return (*(*m_it).second); }
		const registeredService* operator->() const { return ((*m_it).second); }

		const_iterator& operator++() { ++m_it; return (*this); }
		const_iterator operator++(int) { return (m_it++); }

		const_iterator& operator--() { --m_it; return (*this); }
		const_iterator operator--(int) { return (m_it--); }

		const bool operator==(const const_iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const const_iterator& it) const { return (m_it != it.m_it); }

	private:

		const_iterator(const ProtoMap::const_iterator it) : m_it(it) { }

		ProtoMap::const_iterator m_it;
	};

	class iterator
	{
		friend class serviceFactory;
		friend class serviceFactory::const_iterator;

	public:

		iterator() { }
		iterator(const iterator& it) : m_it(it.m_it) { }

		iterator& operator=(const iterator& it) { m_it = it.m_it; return (*this); }

		registeredService& operator*() const { return (*(*m_it).second); }
		registeredService* operator->() const { return ((*m_it).second); }

		iterator& operator++() { ++m_it; return (*this); }
		iterator operator++(int) { return (m_it++); }

		iterator& operator--() { --m_it; return (*this); }
		iterator operator--(int) { return (m_it--); }

		const bool operator==(const iterator& it) const { return (m_it == it.m_it); }
		const bool operator!=(const iterator& it) const { return (m_it != it.m_it); }

	private:

		iterator(const ProtoMap::iterator it) : m_it(it) { }

		ProtoMap::iterator m_it;
	};

	iterator begin() { return iterator(m_protoMap.begin()); }
	iterator end() { return iterator(m_protoMap.end()); }

	const_iterator begin() const { return const_iterator(m_protoMap.begin()); }
	const_iterator end() const { return const_iterator(m_protoMap.end()); }
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_SERVICEFACTORY_HPP_INCLUDED
