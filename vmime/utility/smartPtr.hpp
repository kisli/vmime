//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

	T* const operator ->() { return (m_ptr); }
	T& operator *() { return (*m_ptr); }
};


/** Null reference.
  */

class null_ref
{
private:

	int foo;
};


/** Strong reference (smart pointer).
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

	virtual ~ref() { detach(); }

	// Allow creating NULL ref (NULL casts to anything*)
	ref(class null_pointer*) : m_ptr(0) { }


	// Access to wrapped object
//	operator const T*() const { return m_ptr; }
	operator const void*() const { return m_ptr; }

	T& operator *() { return *m_ptr; }
	const T& operator *() const { return *m_ptr; }

	T* operator ->() { return m_ptr; }
	const T* operator ->() const { return m_ptr; }

	const T* const get() const { return m_ptr; }
	T* const get() { return m_ptr; }


	// dynamic_cast
	template <class U>
	ref <U> dynamicCast() const
	{
		U* p = dynamic_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return ref <U>();

		p->addStrong();

		ref <U> r;
		r.m_ptr = p;

		return r;
	}

	// static_cast
	template <class U>
	ref <U> staticCast() const
	{
		U* p = static_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return ref <U>();

		p->addStrong();

		ref <U> r;
		r.m_ptr = p;

		return r;
	}

	// const_cast
	template <class U>
	ref <U> constCast() const
	{
		U* p = const_cast <U*>(m_ptr);
		if (!p) return ref <U>();

		m_ptr->addStrong();

		ref <U> r;
		r.m_ptr = p;

		return r;
	}

	// Implicit downcast
	template <class U>
	operator ref <const U>() const
	{
		if (m_ptr)
			m_ptr->addStrong();

		ref <const U> r;
		r.m_ptr = m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	template <class U>
	operator ref <U>()
	{
		if (m_ptr)
			m_ptr->addStrong();

		ref <U> r;
		r.m_ptr = m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	template <class U>
	ref <T>& operator=(const ref <U>& other)
	{
		U* ptr = other.m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		if (ptr)
			ptr->addStrong();

		detach();

		m_ptr = ptr;

		return *this;
	}

	// Implicit non-const => const conversion
	operator ref <const T>() const
	{
		if (m_ptr)
			m_ptr->addStrong();

		ref <const T> r;
		r.m_ptr = m_ptr;

		return r;
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
		if (ptr)
			ptr->addStrong();

		ref <T> r;
		r.m_ptr = ptr;

		return r;
	}

protected:

	void detach()
	{
		if (m_ptr)
		{
			m_ptr->releaseStrong();
			m_ptr = 0;
		}
	}

	template <class U>
	void attach(U* const ptr)
	{
		if (ptr)
			ptr->addStrong();

		detach();

		m_ptr = ptr;
	}

	template <class U>
	void attach(const ref <U>& r)
	{
		if (r.m_ptr)
			r.m_ptr->addStrong();

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



/** Base class for weak references.
  */

class weak_ref_base
{
	friend class vmime::object;  // calls 'notifyObjectDestroyed'

protected:

	weak_ref_base() { }
	weak_ref_base(const weak_ref_base&) { }
	virtual ~weak_ref_base() { }


	virtual void notifyObjectDestroyed() = 0;
};


/** Weak reference.
  * Avoid circular references.
  */

template <class T>
class weak_ref : public weak_ref_base
{
public:

	template <class U> friend class weak_ref;


	weak_ref() : m_ptr(0) { }
	weak_ref(const ref <T>& r) : m_ptr(0) { attach(r); }
	weak_ref(const weak_ref& r) : weak_ref_base(), m_ptr(0) { attach(r); }
	weak_ref(const null_ref&) : m_ptr(0) { }
	weak_ref(T* const p) : m_ptr(0) { attach(p); }

	~weak_ref() { detach(); }


	// Access to wrapped object
//	operator const T*() const { return m_ptr; }
	operator const void*() const { return m_ptr; }

	T& operator *() { return *m_ptr; }
	const T& operator *() const { return *m_ptr; }

	T* operator ->() { return m_ptr; }
	const T* operator ->() const { return m_ptr; }

	const T* const get() const { return m_ptr; }
	T* const get() { return m_ptr; }

	const bool operator !() const { return m_ptr == NULL; }


	// dynamic_cast
	template <class U>
	weak_ref <U> dynamicCast() const
	{
		U* p = dynamic_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return weak_ref <U>();

		weak_ref <U> r;

		p->addWeak(&r);

		r.m_ptr = p;

		return r;
	}

	// static_cast
	template <class U>
	weak_ref <U> staticCast() const
	{
		U* p = static_cast <U*>(const_cast <T*>(m_ptr));
		if (!p) return weak_ref <U>();

		weak_ref <U> r;

		p->addWeak(&r);

		r.m_ptr = p;

		return r;
	}

	// const_cast
	template <class U>
	weak_ref <U> constCast() const
	{
		U* p = const_cast <U*>(m_ptr);
		if (!p) return weak_ref <U>();

		weak_ref <U> r;

		p->addWeak(&r);

		r.m_ptr = p;

		return r;
	}

	// Implicit downcast
	template <class U>
	operator weak_ref <const U>()
	{
		weak_ref <const U> r;

		if (m_ptr)
			m_ptr->addWeak(&r);

		r.m_ptr = m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	// Implicit downcast
	template <class U>
	operator weak_ref <U>()
	{
		weak_ref <U> r;

		if (m_ptr)
			m_ptr->addWeak(&r);

		r.m_ptr = m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		return r;
	}

	template <class U>
	weak_ref <T>& operator=(const weak_ref <U>& other)
	{
		U* ptr = other.m_ptr;   // will type check at compile-time (prevent from implicit upcast)

		if (ptr)
			ptr->addWeak(this);

		detach();

		m_ptr = ptr;

		return *this;
	}

	// Implicit non-const => const conversion
	operator weak_ref <const T>() const
	{
		weak_ref <const T> r;

		if (m_ptr)
			m_ptr->addWeak(&r);

		r.m_ptr = m_ptr;

		return r;
	}

	template <class U>
	operator weak_ref <const U>() const
	{
		weak_ref <const U> r;

		if (m_ptr)
			m_ptr->addWeak(&r);

		r.m_ptr = m_ptr;

		return r;
	}

	// Copy
	weak_ref& operator=(const weak_ref& p)
	{
		attach(p);
		return *this;
	}

	// NULL-pointer comparison
	bool operator==(const class null_pointer*) const { return m_ptr == 0; }
	bool operator!=(const class null_pointer*) const { return m_ptr != 0; }

private:

	void notifyObjectDestroyed()
	{
		m_ptr = 0;
	}

	void detach()
	{
		if (m_ptr)
		{
			m_ptr->releaseWeak(this);
			m_ptr = 0;
		}
	}

	void attach(const ref <T>& r)
	{
		if (r.m_ptr)
			r.m_ptr->addWeak(this);

		detach();

		m_ptr = r.m_ptr;
	}

	void attach(const weak_ref& r)
	{
		if (r.m_ptr)
			r.m_ptr->addWeak(this);

		detach();

		m_ptr = r.m_ptr;
	}

	void attach(T* const p)
	{
		if (p)
			p->addWeak(this);

		detach();

		m_ptr = p;
	}


	T* m_ptr;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SMARTPTR_HPP_INCLUDED
