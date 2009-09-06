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

#include "tests/testUtils.hpp"

#include "vmime/utility/smartPtr.hpp"


#define VMIME_TEST_SUITE         smartPtrTest
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testNull)
		VMIME_TEST(testRefCounting)
		VMIME_TEST(testWeakRef)
		VMIME_TEST(testCast)
		VMIME_TEST(testContainer)
		VMIME_TEST(testCompare)
	VMIME_TEST_LIST_END


	struct A : public vmime::object
	{
		int strongCount() const { return getRefManager()->getStrongRefCount(); }
		int weakCount() const { return getRefManager()->getWeakRefCount(); }
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

		VASSERT("1", r1 == NULL);
		VASSERT("2", r1 == 0);
		VASSERT("3", NULL == r1);
		VASSERT("4", 0 == r1);
		VASSERT("5", !r1);
		VASSERT("6", r1 == vmime::null);
		VASSERT("7", vmime::null == r1);
		VASSERT_EQ("8", static_cast <A*>(0), r1.get());
	}

	void testRefCounting()
	{
		bool o1_alive;
		vmime::ref <R> r1 = vmime::create <R>(&o1_alive);

		VASSERT("1", r1.get() != 0);
		VASSERT("2", o1_alive);
		VASSERT_EQ("3", 1, r1->strongCount());
		VASSERT_EQ("4", 1, r1->weakCount());

		vmime::ref <R> r2 = r1;

		VASSERT("5", o1_alive);
		VASSERT_EQ("6", 2, r1->strongCount());
		VASSERT_EQ("7", 2, r1->weakCount());

		bool o2_alive;
		vmime::ref <R> r3 = vmime::create <R>(&o2_alive);

		r2 = r3;

		VASSERT("8", o1_alive);
		VASSERT("9", o2_alive);
		VASSERT_EQ("10", 1, r1->strongCount());
		VASSERT_EQ("11", 2, r2->strongCount());
		VASSERT_EQ("12", 2, r3->strongCount());

		{
			vmime::ref <R> r4;

			r4 = r1;

			VASSERT("13", o1_alive);
			VASSERT("14", o2_alive);
			VASSERT_EQ("15", 2, r4->strongCount());
			VASSERT_EQ("16", 2, r1->strongCount());

			r1 = NULL;

			VASSERT("17", o1_alive);
			VASSERT("18", o2_alive);
			VASSERT_EQ("19", 1, r4->strongCount());

			// Here, object1 will be deleted
		}

		VASSERT("20", !o1_alive);
		VASSERT("21", o2_alive);

		{
			vmime::weak_ref <R> w1 = r3;

			VASSERT_EQ("22", 3, r3->weakCount());
		}

		VASSERT("23", o2_alive);
		VASSERT_EQ("24", 2, r3->strongCount());
		VASSERT_EQ("25", 2, r3->weakCount());
	}

	void testWeakRef()
	{
		vmime::ref <A> r1 = vmime::create <A>();
		vmime::weak_ref <A> w1 = r1;

		VASSERT("1", r1.get() != 0);
		VASSERT("2", r1.get() == w1.acquire().get());

		{
			vmime::ref <A> r2 = r1;

			VASSERT("3", r1.get() == r2.get());
			VASSERT("4", r1.get() == w1.acquire().get());
		}

		VASSERT("5", r1.get() != 0);
		VASSERT("6", r1.get() == w1.acquire().get());

		r1 = 0;

		VASSERT("7", w1.acquire().get() == 0);
	}

	void testCast()
	{
		// Explicit upcast
		vmime::ref <A> r1 = vmime::create <C>();
		vmime::ref <C> r2 = r1.dynamicCast <C>();

		VASSERT("1", r2.get() == dynamic_cast <C*>(r1.get()));
		VASSERT("2", 0 == r1.dynamicCast <B>().get());

		// Implicit downcast
		vmime::ref <D> r3 = vmime::create <D>();
		vmime::ref <A> r4 = r3;

		VASSERT("3", r4.get() == dynamic_cast <A*>(r3.get()));
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

		VASSERT("1", o1_alive);
		VASSERT_EQ("2", 2, r1->strongCount());
		VASSERT("3", o2_alive);
		VASSERT_EQ("4", 2, r2->strongCount());

		{
			std::vector <vmime::ref <R> > v2 = v1;

			VASSERT("5", o1_alive);
			VASSERT_EQ("6", 3, r1->strongCount());
			VASSERT("7", o2_alive);
			VASSERT_EQ("8", 3, r2->strongCount());

			v2[1] = NULL;

			VASSERT("9", o1_alive);
			VASSERT_EQ("10", 3, r1->strongCount());
			VASSERT("11", o2_alive);
			VASSERT_EQ("12", 2, r2->strongCount());
		}

		VASSERT("13", o1_alive);
		VASSERT_EQ("14", 2, r1->strongCount());
		VASSERT("15", o2_alive);
		VASSERT_EQ("16", 2, r2->strongCount());
	}

	void testCompare()
	{
		vmime::ref <A> r1 = vmime::create <A>();
		vmime::ref <A> r2 = vmime::create <B>();
		vmime::ref <A> r3 = vmime::create <C>();
		vmime::ref <A> r4 = r1;

		VASSERT("1", r1 != r2);
		VASSERT("2", r1.get() == r1);
		VASSERT("3", r1 == r1.get());
		VASSERT("4", r2 != r1.get());
		VASSERT("5", r1.get() != r2);
		VASSERT("6", r1 == r4);
		VASSERT("7", r1.get() == r4);

		std::vector <vmime::ref <A> > v;
		v.push_back(r1);
		v.push_back(r2);

		VASSERT("8", std::find(v.begin(), v.end(), r1) == v.begin());
		VASSERT("9", std::find(v.begin(), v.end(), r2) == v.begin() + 1);
		VASSERT("10", std::find(v.begin(), v.end(), r3) == v.end());
	}

VMIME_TEST_SUITE_END

