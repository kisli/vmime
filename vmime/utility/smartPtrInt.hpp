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

#ifndef VMIME_UTILITY_SMARTPTRIMPL_HPP_INCLUDED
#define VMIME_UTILITY_SMARTPTRIMPL_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/utility/smartPtr.hpp"


namespace vmime {
namespace utility {


/** Reference counter for shared pointers.
  */

class refCounter
{
public:

	refCounter(const long initialValue);
	~refCounter();

	long increment();
	long decrement();

	operator long() const;

private:

#if defined(_WIN32)
	long m_value;
#elif defined(__GNUC__) && (defined(__GLIBCPP__) || defined(__GLIBCXX__))
	mutable volatile int m_value;
#elif defined (VMIME_HAVE_PTHREAD)
	volatile long m_value;
	pthread_mutex_t m_mutex;
#else // not thread-safe implementation
	long m_value;
#endif

};


/** Separate implementation of refManager, to avoid polluting global
  * namespace with system-specific inclusions/definitions.
  */

class refManagerImpl : public refManager
{
public:

	refManagerImpl(object* obj);
	~refManagerImpl();

	bool addStrong();
	void releaseStrong();

	void addWeak();
	void releaseWeak();

	object* getObject();

	long getStrongRefCount() const;
	long getWeakRefCount() const;

private:

	void deleteManager();
	void deleteObject();


	object* m_object;

	refCounter m_strongCount;
	refCounter m_weakCount;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SMARTPTRIMPL_HPP_INCLUDED

