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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_UTILITY_SMARTPTR_HPP_INCLUDED
#define VMIME_UTILITY_SMARTPTR_HPP_INCLUDED


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


/** Smart auto-delete, referencable and copiable pointer.
  */

template <class T>
class smart_ptr
{
private:

	struct data
	{
		int refCount;
		T* ptr;
	};

	data* m_data;


	typedef std::map <T*, data*> MapType;
	static MapType sm_map;

public:

	smart_ptr() : m_data(NULL) { }
	smart_ptr(T* const p) : m_data(NULL) { if (p) { attach(p); } }
	smart_ptr(smart_ptr& p) : m_data(NULL) { if (p.m_data) { attach(p); } }

	~smart_ptr() { detach(); }

	smart_ptr& operator=(smart_ptr& p)
	{
		attach(p);
		return (*this);
	}

	smart_ptr& operator=(T* const p)
	{
		if (!p)
			detach();
		else
			attach(p);

		return (*this);
	}

	operator T*() { return (m_data ? m_data->ptr : NULL); }
	operator const T*() { return (m_data ? m_data->ptr : NULL); }

	T& operator *() { return (*(m_data->ptr)); }
	T* operator ->() { return (m_data->ptr); }

	const T* const ptr() const { return (m_data ? m_data->ptr : NULL); }
	T* const ptr() { return (m_data ? m_data->ptr : NULL); }

private:

	void detach()
	{
		if (m_data)
		{
			if (m_data->refCount == 1)
			{
				typename MapType::iterator it = sm_map.find(m_data->ptr);
				if (it != sm_map.end()) sm_map.erase(it);

				delete (m_data->ptr);
				delete (m_data);
			}
			else
			{
				m_data->refCount--;
			}

			m_data = NULL;
		}
	}

	void attach(T* const p)
	{
		detach();

		typename MapType::iterator it = sm_map.find(p);

		if (it != sm_map.end())
		{
			(*it).second->refCount++;
		}
		else
		{
			m_data = new data;
			m_data->refCount = 1;
			m_data->ptr = p;

			sm_map.insert(typename MapType::value_type(p, m_data));
		}
	}

	void attach(smart_ptr <T>& p)
	{
		data* newData = p.m_data;
		if (newData) newData->refCount++;

		detach();

		m_data = newData;
	}
};


template <class T>
typename smart_ptr <T>::MapType smart_ptr <T>::sm_map;


} // utility
} // vmime


#endif // VMIME_UTILITY_SMARTPTR_HPP_INCLUDED
