// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include "unit++.h"
#include "optmap.h"
#include <iostream>
#ifdef HAVE_SSTREAM
#include <sstream>
#endif
using namespace std;
using namespace unitpp;
using namespace options_utils;
namespace {
const char* x[] = { "testing", "-i", "120", "-n100", "-t" };
#ifdef HAVE_SSTREAM
class hijack {
	ostream& os;
	streambuf* sbp;
	stringbuf sb;
public:
	hijack(ostream& os)
	: os(os), sbp(os.rdbuf())
	{
		os.rdbuf(&sb);
	}
	~hijack() { os.rdbuf(sbp); }
	string str() { return sb.str(); }
};
#endif
// The test suite for the optmap library
class Test : public suite
{
	const char** argv;
	int argc;
	void create()
	{
		optmap om;
	}
	void usage()
	{
		bool t_flg;
		int i = 7;
		int n = 0;
		optmap om;
		om.add("t", new opt_flag(t_flg));
		om.add("i", new opt_int(i));
		om.add("n", new opt_int(n));
		om.alias("number", "n");
#ifdef HAVE_SSTREAM
		string exp(
			"usage: testing [ -t ] [ -i <int> ] [( -n | --number) <int> ]\n");
		{
			hijack s(cerr);
			om.parse(argc, argv);
			om.usage(false);
			assert_eq("usage", exp, s.str());
		}
#endif
	}
	void args()
	{
		bool t_flg = false;
		int i = 7;
		int n = 0;
		optmap om;
		om.add("t", new opt_flag(t_flg));
		om.add("i", new opt_int(i));
		om.add("n", new opt_int(n));
		assert_true("parse ok", om.parse(argc, argv));
		assert_eq("get -i", 120, i);
		assert_eq("get -n", 100, n);
		assert_eq("got -t", true, t_flg);
	}
	void tail()
	{
		const char* argv[] = { "xyzzy", "-abc", "-def", "hij" };
		size_t argc = sizeof(argv)/sizeof(argv[0]);
		string s;
		bool f_d, f_e, f_f;
		optmap om;
		om.add("a", new opt_string(s));
		om.add("d", new opt_flag(f_d));
		om.add("e", new opt_flag(f_e));
		om.add("f", new opt_flag(f_f));
		assert_true("parse", om.parse(static_cast<int>(argc), argv));
		assert_eq("n", argc - 1, size_t(om.n()));
		assert_eq("a", "bc", s);
	}
public:
	Test() : suite("optmap test suite")
	{
		argv = x;
		argc = sizeof(x)/sizeof(char*);
		suite::main().add("optmap", this);
		add("create", testcase(this, "Create an optmap", &Test::create));
		add("usage", testcase(this, "Optmap usage", &Test::usage));
		add("args", testcase(this, "Optmap args", &Test::args));
		add("tail", testcase(this, "Optmap tail", &Test::tail));
	}
} * theTest = new Test();

}
