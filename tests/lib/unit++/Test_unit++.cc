// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include "unit++.h"
#include "tester.h"
#include "main.h"
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <iostream>
#endif
using namespace std;
using namespace unitpp;
namespace {

// a test case that can fail with any exception
class test_test : public test
{
public:
	enum result { succes, fail, error, exotic };
	test_test(string name, result res = succes) : test(name), res(res) {}
	virtual void operator()()
	{
		switch (res) {
		case succes: break;
		case fail: ::fail("test_test");
		case error: throw out_of_range("ranged");
		case exotic: throw 4711;
		}
	}
private:
	result res;
};

// The test suite for the unit++ library
class Test : public suite
{
	void create()
	{
		test_test a_loc_test("local");
	}
	void assert_ok()
	{
		string s("ok");
		assert_true("assert_true(true)", true);
		assert_eq("assert_eq(int)", 7, 7);
		assert_eq("assert_eq(char*, string)", "ok", s);
	}
	void assert_fail()
	{
		string s("fejl");
		bool ok = true;
		try {
			assert_true("assert_true(false)", false);
			ok = false;
		} catch (assertion_error e) {
#ifdef HAVE_SSTREAM
			ostringstream oss;
			oss << e;
			assert_eq("assert_true(false) output",
				"assert_true(false) [assertion failed]", oss.str());
#endif
		}
		if (!ok)
			fail("no exception from assert_true(false)");
		try {
			assert_eq("assert_eq(int)", 5, 7);
			ok = false;
		} catch (assert_value_error<int,int> e) {
#ifdef HAVE_SSTREAM
			ostringstream oss;
			oss << e;
			assert_eq("assert_eq(int) output",
				"assert_eq(int) [expected: `5' got: `7']", oss.str());
#endif
		}
		if (!ok)
			fail("no exception from assert_eq(int)");
		try {
			assert_eq("assert_eq(char*, string)", "ok", s);
			ok = false;
		} catch (assert_value_error<const char*, string> e) {
		} catch (assert_value_error<char*, string> e) { // MSVC++ bug
		}

		if (!ok)
			fail("no exception from assert_eq(const char*, string)");
	}
	void tester_visit()
	{
		out_of_range oor("negative");
		assertion_error ae("test");
#ifdef HAVE_SSTREAM
		ostringstream os;
		tester tst(os);
#else
		tester tst(cerr);
#endif
		root.visit(&tst);
		assert_eq("tests ok", 3, tst.res_tests().n_ok());
		assert_eq("tests error", 2, tst.res_tests().n_err());
		assert_eq("tests fail", 1, tst.res_tests().n_fail());
		assert_eq("suites ok", 1, tst.res_suites().n_ok());
		assert_eq("suites error", 2, tst.res_suites().n_err());
		assert_eq("suites fail", 1, tst.res_suites().n_fail());
	}
	void ex_test()
	{
		throw out_of_range("expected");
	}
	void get_by_id()
	{
		test* p = root.get_child("s2");
		assert_true("found s2", p != 0);
		suite* sp = dynamic_cast<suite*>(p);
		assert_true("s2 was suite", sp != 0);
		assert_eq("right s2", "S2", sp->name());
		p = sp->get_child("t20");
		assert_true("found t20", p != 0);
		assert_eq("not suite", static_cast<suite*>(0),dynamic_cast<suite*>(p));
	}
	void vec()
	{
		string s = "three.blind.mice";
		vector<string> v(vectorize(s,'.'));
		assert_eq("v[0]", string("three"), v[0]);
		assert_eq("v[1]", string("blind"), v[1]);
		assert_eq("v[2]", string("mice"), v[2]);
		assert_eq("size", size_t(3), v.size());
		v = vectorize(s,'-');
		assert_eq("no match", s, v[0]);
		assert_eq("no match size", size_t(1), v.size());
	}
	void empty_vec()
	{
		string s("");
		vector<string> v(vectorize(s,'.'));
		assert_eq("size", size_t(0), v.size());
		s = "one..three";
		v = vectorize(s,'.');
		assert_eq("v[0]", string("one"), v[0]);
		assert_eq("v[1]", string(""), v[1]);
		assert_eq("v[2]", string("three"), v[2]);
		assert_eq("size", size_t(3), v.size());
	}
	void find()
	{
		test* tp = root.find("s2.s21.t210");
		assert_eq("t210", t210, tp);
		tp = root.find("s1.s21");
		assert_eq("bad mid", static_cast<test*>(0), tp);
	}
	suite root;
	test* t210;
	bool do_fail;
	void fail_on_flag()
	{
		assert_true("Fail option not set", !do_fail);
	}
public:
	Test() : suite("Unit++ test suite"), root("The root")
	{
		do_fail = false;
		options().add("f", new options_utils::opt_flag(do_fail));
		options().alias("fail", "f");
		suite* s1;
		suite* s2;
		suite* s21;
		root.add("s1", s1 = new suite("S1"));
		root.add("s2", s2 = new suite("S2"));
		s2->add("s21", s21 = new suite("S21"));
		s1->add("t10", new test_test("T10"));
		s1->add("t11", new test_test("T11"));
		s2->add("t20", new test_test("T20", test_test::error));
		s2->add("t22", new test_test("T22", test_test::exotic));
		s21->add("t210", t210 = new test_test("T210"));
		s21->add("t211", new test_test("T211", test_test::fail));
		// 
		// Adding testcases
		suite::main().add("unitpp", this);
		add("create", testcase(this, "Create a test", &Test::create));
		add("assert_ok", testcase(this, "Assert ok", &Test::assert_ok));
		add("assert_fail", testcase(this, "Assert fail", &Test::assert_fail));
		add("tester_visit", testcase(this, "Visit", &Test::tester_visit));
		add("exception", testcase(new exception_test<out_of_range>(
			testcase(this, "gen ex", &Test::ex_test))));
		add("id_get", testcase(this, "Get by id", &Test::get_by_id));
		add("vec", testcase(this, "Vectorize", &Test::vec));
		add("empty_vec", testcase(this, "Vectorize empty", &Test::empty_vec));
		add("find", testcase(this, "find", &Test::find));
		add("fail", testcase(this, "fail on option", &Test::fail_on_flag));
	}
} * theTest = new Test();

}
