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

#ifndef VMIME_UTILITY_SMARTPTR_HPP_INCLUDED
#define VMIME_UTILITY_SMARTPTR_HPP_INCLUDED


#include <map>


// Forward reference to 'object'
namespace vmime { class object; }


namespace vmime {
namespace utility {


/** Simple auto-delete pointer.
  */

template <class T>
class auto_ptr
{
private:

	T* const m_ptr;

public:

	auto_ptr(T* const ptr) : m_ptr(ptr) { }
	~auto_ptr() { delete (m_ptr); }

	operator T*() { return (m_ptr); }

	T* operator ->() { return (m_ptr); }
	T& operator *() { return (*m_ptr); }
};


/** Manage the life cycle of an object.
  */

class refManager
{
protected:

	refManager() {}

public:

	virtual ~refManager() {}

	/** Create a ref manager for the specified object.
	  *
	  * @return a new manager
	  */
	static refManager* create(object* obj);

	/** Add a strong reference to the managed object.
	  */
	virtual bool addStrong() = 0;

	/** Release a strong reference to the managed object.
	  * If it is the last reference, the object is destroyed.
	  */
	virtual void releaseStrong() = 0;

	/** Add a weak reference to the managed object.
	  */
	virtual void addWeak() = 0;

	/** Release a weak reference to the managed object.
	  * If it is the last weak reference, the manager is destroyed.
	  */
	virtual void releaseWeak() = 0;

	/** Return a raw pointer to the managed object.
	  *
	  * @return pointer to the managed object
	  */
	virtual object* getObject() = 0;

	/** Return the number of strong refs to this object.
	  * For debugging purposes only.
	  *
	  * @return strong reference count
	  */
	virtual long getStrongRefCount() const = 0;

	/** Return the number of weak refs to this object.
	  * For debugging purposes only.
	  *
	  * @return weak reference count
	  */
	virtual long getWeakRefCount() const = 0;

protected:

	void deleteObjectImpl(object* obj);
};


/** Null reference.
  */

class null_ref
{
private:

	int foo;
};


template <class T>
class weak_ref;


/** Shared ownership (strong reference to an object).
  */

template <class T>
class ref
{
public:

	template <class U> friend class ref;
	template <class U> friend class weak_ref;


	ref() : m_ptr(0) { }
	ref(const ref& r) : m_ptr(0) { attach(r); }
	ref(const null_ref&) : m_ptr(0) { }

	virtual ~ref() throw() { detach(); }

	// Allow creating NULL ref (NULL casts to anything*)
	ref(class null_pointer*) : m_ptr(0) { }


	// Access to wrapped object
//	operator const T*() const { return m_ptr; }
	operator const void*() const { return m_ptr; }

	T& operator *() { return *m_ptr; }
	const T& operator *() const { return *m_ptr; }

	T* operator ->() { return m_ptr; }
	const T* operator ->() const { return m_ptr; }

	const T* get() const { return m_ptr; }
	T* get() { return m_ptr; }


	// dynamic_cast
	template <class U>
	ref <U> dynamicCast() const
	{
		U* p = dynamic_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return ref <U>();

		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

		return ref <U>::fromPtrImpl(p);
	}

	// static_cast
	template <class U>
	ref <U> staticCast() const
	{
		U* p = static_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return ref <U>();

		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

		return ref <U>::fromPtrImpl(p);
	}

	// const_cast
	template <class U>
	ref <U> constCast() const
	{
		U* p = const_cast <U*>(m_ptr);
		if (!p) return ref <U>();

		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

		return ref <U>::fromPtrImpl(p);
	}

	// Implicit downcast
	template <class U>
	operator ref <const U>() const
	{
		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

		ref <const U> r;
		r.m_ptr = m_ptr; // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	template <class U>
	operator ref <U>()
	{
		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

		ref <U> r;
		r.m_ptr = m_ptr; // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	template <class U>
	ref <T>& operator=(const ref <U>& other)
	{
		U* ptr = other.m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		if (ptr)
			ptr->getRefManager()->addStrong();

		detach();

		m_ptr = ptr;

		return *this;
	}

	// Implicit non-const => const conversion
	operator ref <const T>() const
	{
		if (m_ptr)
			m_ptr->getRefManager()->addStrong();

#if defined(_MSC_VER) // VC++ compiler bug (stack overflow)
		ref <const T> r;
		r.m_ptr = m_ptr;
		return r;
#else
		return ref <const T>::fromPtrImpl(m_ptr);
#endif // defined(_MSC_VER)

	}

	// Copy
	ref& operator=(const ref& p)
	{
		attach(p);
		return *this;
	}

	// NULL-pointer comparison
	bool operator==(const class null_pointer*) const { return m_ptr == 0; }
	bool operator!=(const class null_pointer*) const { return m_ptr != 0; }

	bool operator==(const null_ref&) const { return m_ptr == 0; }
	bool operator!=(const null_ref&) const { return m_ptr != 0; }

	/** Create a ref<> from a raw pointer.
	  *
	  * WARNING: you should use this function only if you know what
	  * you are doing. In general, you should create ref objects using
	  * vmime::create().
	  *
	  * When this function returns, the pointer is owned by the ref,
	  * you should not attempt to delete it manually.
	  *
	  * @param ptr raw pointer to encapsulate
	  * @return a ref which encapsulates the specified raw pointer
	  */
	static ref <T> fromPtr(T* const ptr)
	{
		return ref <T>::fromPtrImpl(ptr);
	}

	static ref <const T> fromPtrConst(const T* const ptr)
	{
		return ref <const T>::fromPtrImpl(ptr);
	}

	static ref <T> fromWeak(weak_ref <T> wr)
	{
		refManager* mgr = wr.getManager();

		if (mgr && mgr->addStrong())
			return ref <T>::fromPtrImpl(dynamic_cast <T*>(mgr->getObject()));
		else
			return ref <T>();
	}

	static ref <const T> fromWeakConst(weak_ref <const T> wr)
	{
		refManager* mgr = wr.getManager();

		if (mgr && mgr->addStrong())
			return ref <const T>::fromPtrImpl(dynamic_cast <const T*>(mgr->getObject()));
		else
			return ref <const T>();
	}

protected:

	template <class U>
	static ref <U> fromPtrImpl(U* ptr)
	{
		ref <U> r;
		r.m_ptr = ptr;

		return r;
	}

	void detach()
	{
		if (m_ptr)
		{
			if (m_ptr->getRefManager())
				m_ptr->getRefManager()->releaseStrong();

			m_ptr = 0;
		}
	}

	template <class U>
	void attach(U* const ptr)
	{
		if (ptr)
			ptr->getRefManager()->addStrong();

		detach();

		m_ptr = ptr;
	}

	template <class U>
	void attach(const ref <U>& r)
	{
		if (r.m_ptr)
			r.m_ptr->getRefManager()->addStrong();

		detach();

		m_ptr = r.m_ptr;
	}

private:

	T* m_ptr;
};



template <class T, class U>
bool operator==(const ref <T>& a, const ref <U>& b)
{
	return (a.get() == b.get());
}

template <class T, class U>
bool operator!=(const ref <T>& a, const ref <U>& b)
{
	return (a.get() != b.get());
}

template <class T>
bool operator==(const ref <T>& a, T* const p)
{
	return (a.get() == p);
}

template <class T>
bool operator!=(const ref <T>& a, T* const p)
{
	return (a.get() != p);
}

template <class T>
bool operator==(T* const p, const ref <T>& a)
{
	return (a.get() == p);
}

template <class T>
bool operator!=(T* const p, const ref <T>& a)
{
	return (a.get() != p);
}

template <class T>
bool operator==(const null_ref&, const ref <T>& r)
{
	return (r.get() == 0);
}

template <class T>
bool operator!=(const null_ref&, const ref <T>& r)
{
	return (r.get() != 0);
}



/** Weak reference.
  * Avoid circular references.
  */

template <class T>
class weak_ref
{
public:

	template <class U> friend class weak_ref;


	weak_ref() : m_mgr(0) { }
	weak_ref(const ref <T>& r) : m_mgr(0) { attach(r); }
	weak_ref(const weak_ref& r) : m_mgr(0) { attach(r); }
	weak_ref(const null_ref&) : m_mgr(0) { }
	weak_ref(class null_pointer*) : m_mgr(0) { }

	~weak_ref() { detach(); }

	/** Return the manager for the object.
	  *
	  * @return pointer to the object which manages the object
	  * or NULL if the weak reference points to nothing
	  */
	refManager* getManager()
	{
		return m_mgr;
	}

	/** Try to acquire a strong reference to the object (const version).
	  *
	  * @return strong reference or null reference if the
	  * object is not available anymore
	  */
	ref <const T> acquire() const
	{
		return ref <const T>::fromWeakConst(*this);
	}

	/** Try to acquire a strong reference to the object.
	  *
	  * @return strong reference or null reference if the
	  * object is not available anymore
	  */
	ref <T> acquire()
	{
		return ref <T>::fromWeak(*this);
	}

	// Implicit non-const => const conversion
	operator weak_ref <const T>() const
	{
		if (m_mgr)
			m_mgr->addWeak();

		weak_ref <const T> r;
		r.m_mgr = m_mgr;

		return r;
	}

	template <class U>
	operator weak_ref <const U>() const
	{
		if (m_mgr)
			m_mgr->addWeak();

		weak_ref <const U> r;
		r.m_mgr = m_mgr;

		return r;
	}

	// Copy
	weak_ref& operator=(const weak_ref& p)
	{
		attach(p);
		return *this;
	}

private:

	void detach()
	{
		if (m_mgr)
		{
			m_mgr->releaseWeak();
			m_mgr = 0;
		}
	}

	void attach(const ref <T>& r)
	{
		if (r.m_ptr)
			r.m_ptr->getRefManager()->addWeak();

		detach();

		if (r.m_ptr)
			m_mgr = r.m_ptr->getRefManager();
		else
			m_mgr = 0;
	}

	void attach(const weak_ref& r)
	{
		if (r.m_mgr)
			r.m_mgr->addWeak();

		detach();

		m_mgr = r.m_mgr;
	}


	refManager* m_mgr;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SMARTPTR_HPP_INCLUDED
