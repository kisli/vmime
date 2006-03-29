//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/object.hpp"
#include "vmime/utility/smartPtr.hpp"


namespace vmime {
namespace utility {


// refManager

refManager::refManager(object* obj)
	: m_object(obj), m_strongCount(1), m_weakCount(1)
{
}


refManager::~refManager()
{
}


const bool refManager::addStrong()
{
	if (m_strongCount <= 0)
		return false;

	m_strongCount.increment();
	m_weakCount.increment();

	return true;
}


void refManager::releaseStrong()
{
	m_strongCount.decrement();

	if (m_strongCount.compareExchange(0, -424242) == 0)  // prevent from adding strong refs later
		deleteObject();

	releaseWeak();
}


void refManager::addWeak()
{
	m_weakCount.increment();
}


void refManager::releaseWeak()
{
	if (m_weakCount.decrement() == 0)
		deleteManager();
}


object* refManager::getObject()
{
	return m_object;
}


void refManager::deleteManager()
{
	delete this;
}


void refManager::deleteObject()
{
	try
	{
		m_object->setRefManager(0);
		delete m_object;
	}
	catch (...)
	{
		// Exception in destructor
	}

	m_object = 0;
}


const long refManager::getStrongRefCount() const
{
	return m_strongCount;
}


const long refManager::getWeakRefCount() const
{
	return m_weakCount;
}



// refCounter

refCounter::refCounter(const long initialValue)
	: m_value(initialValue)
{
}


refCounter::~refCounter()
{
}


const long refCounter::increment()
{
	return ++m_value;
}


const long refCounter::decrement()
{
	return --m_value;
}


const long refCounter::compareExchange(const long compare, const long exchangeWith)
{
	const int prev = m_value;

	if (m_value == compare)
		m_value = exchangeWith;

	return prev;
}


refCounter::operator long() const
{
	return m_value;
}


} // utility
} // vmime

