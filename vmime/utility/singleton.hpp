//
// VMime library (http://vmime.sourceforge.net)
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

#ifndef VMIME_UTILITY_SINGLETON_HPP_INCLUDED
#define VMIME_UTILITY_SINGLETON_HPP_INCLUDED


#include <list>


namespace vmime {
namespace utility {


// Singleton abstract base class.

class abstractSingleton
{
	friend class singletonManager;

protected:

	abstractSingleton() { }
	virtual ~abstractSingleton() { }
};


// Singleton manager
// (for automatic clean-up of all instanciated singletons).

class singletonManager
{
public:

	static singletonManager* getInstance();

	void manage(abstractSingleton* s);

private:

	singletonManager();
	~singletonManager();

	std::list <abstractSingleton*> m_list;
};


// A singleton template.

template <class TYPE>
class singleton : public abstractSingleton
{
protected:

	singleton() { }
	~singleton() { }

public:

	static TYPE* getInstance()
	{
		static TYPE* inst = NULL;

		if (!inst)
			singletonManager::getInstance()->manage(inst = new TYPE());

		return (inst);
	}
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SINGLETON_HPP_INCLUDED
