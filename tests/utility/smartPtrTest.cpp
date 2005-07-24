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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>
#include <vector>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "vmime/utility/smartPtr.hpp"

using namespace unitpp;


namespace
{
	class smartPtrTest : public suite
	{
		struct A : public vmime::object
		{
			const int strongCount() const { return getStrongRefCount(); }
			const int weakCount() const { return getWeakRefCount(); }
		};

		struct B : public virtual A { };
		struct C : public virtual A { };
		struct D : public B, public C { };

		class R : public A
		{
		public:

			R(bool* aliveFlag) : m_aliveFlag(aliveFlag) { *m_aliveFlag = true; }
			~R() { *m_aliveFlag = false; }

		private:

			bool* m_aliveFlag;
		};


		void testNull()
		{
			vmime::ref <A> r1;

			assert_true("1", r1 == NULL);
			assert_true("2", r1 == 0);
			assert_true("3", NULL == r1);
			assert_true("4", 0 == r1);
			assert_true("5", !r1);
			assert_true("6", r1 == vmime::null);
			assert_true("7", vmime::null == r1);
			assert_eq("8", static_cast <A*>(0), r1.get());
		}

		void testRefCounting()
		{
			bool o1_alive;
			vmime::ref <R> r1 = vmime::create <R>(&o1_alive);

			assert_true("1", r1.get() != 0);
			assert_true("2", o1_alive);
			assert_eq("3", 1, r1->strongCount());
			assert_eq("4", 0, r1->weakCount());

			vmime::ref <R> r2 = r1;

			assert_true("5", o1_alive);
			assert_eq("6", 2, r1->strongCount());
			assert_eq("7", 0, r1->weakCount());

			bool o2_alive;
			vmime::ref <R> r3 = vmime::create <R>(&o2_alive);

			r2 = r3;

			assert_true("8", o1_alive);
			assert_true("9", o2_alive);
			assert_eq("10", 1, r1->strongCount());
			assert_eq("11", 2, r2->strongCount());
			assert_eq("12", 2, r3->strongCount());

			{
				vmime::ref <R> r4;

				r4 = r1;

				assert_true("13", o1_alive);
				assert_true("14", o2_alive);
				assert_eq("15", 2, r4->strongCount());
				assert_eq("16", 2, r1->strongCount());

				r1 = NULL;

				assert_true("17", o1_alive);
				assert_true("18", o2_alive);
				assert_eq("19", 1, r4->strongCount());

				// Here, object1 will be deleted
			}

			assert_true("20", !o1_alive);
			assert_true("21", o2_alive);

			{
				vmime::weak_ref <R> w1 = r3;

				assert_eq("22", 1, r3->weakCount());
			}

			assert_true("23", o2_alive);
			assert_eq("24", 2, r3->strongCount());
			assert_eq("25", 0, r3->weakCount());
		}

		void testWeakRef()
		{
			vmime::ref <A> r1 = vmime::create <A>();
			vmime::weak_ref <A> w1 = r1;

			assert_true("1", r1.get() != 0);
			assert_true("2", r1.get() == w1.get());

			{
				vmime::ref <A> r2 = r1;

				assert_true("3", r1.get() == r2.get());
				assert_true("4", r1.get() == w1.get());
			}

			assert_true("5", r1.get() != 0);
			assert_true("6", r1.get() == w1.get());

			r1 = 0;

			assert_true("7", w1.get() == 0);
		}

		void testCast()
		{
			// Explicit upcast
			vmime::ref <A> r1 = vmime::create <C>();
			vmime::ref <C> r2 = r1.dynamicCast <C>();

			assert_true("1", r2.get() == dynamic_cast <C*>(r1.get()));
			assert_true("2", 0 == r1.dynamicCast <B>().get());

			// Implicit downcast
			vmime::ref <D> r3 = vmime::create <D>();
			vmime::ref <A> r4 = r3;

			assert_true("3", r4.get() == dynamic_cast <A*>(r3.get()));
		}

		void testContainer()
		{
			bool o1_alive;
			vmime::ref <R> r1 = vmime::create <R>(&o1_alive);

			bool o2_alive;
			vmime::ref <R> r2 = vmime::create <R>(&o2_alive);

			std::vector <vmime::ref <R> > v1;
			v1.push_back(r1);
			v1.push_back(r2);

			assert_true("1", o1_alive);
			assert_eq("2", 2, r1->strongCount());
			assert_true("3", o2_alive);
			assert_eq("4", 2, r2->strongCount());

			{
				std::vector <vmime::ref <R> > v2 = v1;

				assert_true("5", o1_alive);
				assert_eq("6", 3, r1->strongCount());
				assert_true("7", o2_alive);
				assert_eq("8", 3, r2->strongCount());

				v2[1] = NULL;

				assert_true("9", o1_alive);
				assert_eq("10", 3, r1->strongCount());
				assert_true("11", o2_alive);
				assert_eq("12", 2, r2->strongCount());
			}

			assert_true("13", o1_alive);
			assert_eq("14", 2, r1->strongCount());
			assert_true("15", o2_alive);
			assert_eq("16", 2, r2->strongCount());
		}

		void testCompare()
		{
			vmime::ref <A> r1 = vmime::create <A>();
			vmime::ref <A> r2 = vmime::create <B>();
			vmime::ref <A> r3 = vmime::create <C>();
			vmime::ref <A> r4 = r1;

			assert_true("1", r1 != r2);
			assert_true("2", r1.get() == r1);
			assert_true("3", r1 == r1.get());
			assert_true("4", r2 != r1.get());
			assert_true("5", r1.get() != r2);
			assert_true("6", r1 == r4);
			assert_true("7", r1.get() == r4);

			std::vector <vmime::ref <A> > v;
			v.push_back(r1);
			v.push_back(r2);

			assert_true("8", std::find(v.begin(), v.end(), r1) == v.begin());
			assert_true("9", std::find(v.begin(), v.end(), r2) == v.begin() + 1);
			assert_true("10", std::find(v.begin(), v.end(), r3) == v.end());
		}

	public:

		smartPtrTest() : suite("vmime::utility::url")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Null", testcase(this, "TestNull", &smartPtrTest::testNull));
			add("RefCounting", testcase(this, "TestRefCounting", &smartPtrTest::testRefCounting));
			add("WeakRef", testcase(this, "TestWeakRef", &smartPtrTest::testWeakRef));
			add("Cast", testcase(this, "TestCast", &smartPtrTest::testCast));
			add("Container", testcase(this, "TestContainer", &smartPtrTest::testContainer));
			add("Compare", testcase(this, "TestCompare", &smartPtrTest::testCompare));

			suite::main().add("vmime::utility::smartPtr", this);
		}

	};

	smartPtrTest* theTest = new smartPtrTest();
}

