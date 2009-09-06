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

#include "vmime/utility/smartPtrInt.hpp"
#include "vmime/object.hpp"

#if defined(_WIN32)
#	include <windows.h>
#elif defined(VMIME_HAVE_PTHREAD)
#	include <pthread.h>
#endif


namespace vmime {
namespace utility {


// static
refManager* refManager::create(object* obj)
{
	return new refManagerImpl(obj);
}


//
// refManager
//

refManagerImpl::refManagerImpl(object* obj)
	: m_object(obj), m_strongCount(1), m_weakCount(1)
{
}


refManagerImpl::~refManagerImpl()
{
}


bool refManagerImpl::addStrong()
{
	if (m_strongCount <= 0)
		return false;

	m_strongCount.increment();
	m_weakCount.increment();

	return true;
}


void refManagerImpl::releaseStrong()
{
	if (m_strongCount.decrement() <= 0)
		deleteObject();

	releaseWeak();
}


void refManagerImpl::addWeak()
{
	m_weakCount.increment();
}


void refManagerImpl::releaseWeak()
{
	if (m_weakCount.decrement() <= 0)
		deleteManager();
}


object* refManagerImpl::getObject()
{
	return m_object;
}


void refManagerImpl::deleteManager()
{
	delete this;
}


void refManagerImpl::deleteObject()
{
	try
	{
		deleteObjectImpl(m_object);
	}
	catch (...)
	{
		// Exception in destructor
	}

	m_object = 0;
}


long refManagerImpl::getStrongRefCount() const
{
	return m_strongCount;
}


long refManagerImpl::getWeakRefCount() const
{
	return m_weakCount;
}



//
// refCounter
//

#ifdef _WIN32


refCounter::refCounter(const long initialValue)
	: m_value(initialValue)
{
}


refCounter::~refCounter()
{
}


long refCounter::increment()
{
	return InterlockedIncrement(&m_value);
}


long refCounter::decrement()
{
	return InterlockedDecrement(&m_value);
}


refCounter::operator long() const
{
	return m_value;
}


#elif defined(__GNUC__) && (defined(__GLIBCPP__) || defined(__GLIBCXX__))


refCounter::refCounter(const long initialValue)
	: m_value(static_cast <int>(initialValue))
{
}


refCounter::~refCounter()
{
}


long refCounter::increment()
{
#if __GNUC_MINOR__ < 4 && __GNUC__ < 4
	return __exchange_and_add(&m_value, 1) + 1;
#else
	return __gnu_cxx::__exchange_and_add(&m_value, 1) + 1;
#endif
}


long refCounter::decrement()
{
#if __GNUC_MINOR__ < 4 && __GNUC__ < 4
	return __exchange_and_add(&m_value, -1) - 1;
#else
	return __gnu_cxx::__exchange_and_add(&m_value, -1) - 1;
#endif
}


refCounter::operator long() const
{
#if __GNUC_MINOR__ < 4 && __GNUC__ < 4
	return static_cast <long>(__exchange_and_add(&m_value, 0));
#else
	return static_cast <long>(__gnu_cxx::__exchange_and_add(&m_value, 0));
#endif
}


#elif defined(VMIME_HAVE_PTHREAD)


refCounter::refCounter(const long initialValue)
	: m_value(initialValue)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&m_mutex, &attr);
	pthread_mutexattr_destroy(&attr);
}


refCounter::~refCounter()
{
	pthread_mutex_destroy(&m_mutex);
}


long refCounter::increment()
{
	long value;

	pthread_mutex_lock(&m_mutex);
	value = ++m_value;
	pthread_mutex_unlock(&m_mutex);

	return value;
}


long refCounter::decrement()
{
	long value;

	pthread_mutex_lock(&m_mutex);
	value = --m_value;
	pthread_mutex_unlock(&m_mutex);

	return value;
}


refCounter::operator long() const
{
	return m_value;
}


#else  // not thread-safe implementation


refCounter::refCounter(const long initialValue)
	: m_value(initialValue)
{
}


refCounter::~refCounter()
{
}


long refCounter::increment()
{
	return ++m_value;
}


long refCounter::decrement()
{
	return --m_value;
}


refCounter::operator long() const
{
	return m_value;
}


#endif


} // utility
} // vmime

