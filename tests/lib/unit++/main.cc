// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include "main.h"
#include <algorithm>
using namespace std;
using namespace unitpp;

bool unitpp::verbose = false;

test_runner* runner = 0;

test_runner::~test_runner()
{
}

void unitpp::set_tester(test_runner* tr)
{
	runner = tr;
}

int main(int argc, const char* argv[])
{
	options().add("v", new options_utils::opt_flag(verbose));
	options().alias("verbose", "v");
	if (!options().parse(argc, argv))
		options().usage();
	plain_runner plain;
	if (!runner)
		runner = &plain;
	return runner->run_tests(argc, argv) ? 0 : 1;
}

namespace unitpp {
options_utils::optmap& options()
{
	static options_utils::optmap opts("[ testids... ]");
	return opts;
}

bool plain_runner::run_tests(int argc, const char** argv)
{
	bool res = true;
	if (options().n() < argc)
		for (int i = options().n(); i < argc; ++i)
			res = res && run_test(argv[i]);
	else
		res = run_test();
	return res;
}

bool plain_runner::run_test(const string& id)
{
	test* tp = suite::main().find(id);
	if (!tp) {
		return false;
	}
	return run_test(tp);
}
bool plain_runner::run_test(test* tp)
{
	tester tst(cout, verbose);
	tp->visit(&tst);
	tst.summary();
	res_cnt res(tst.res_tests());
	return res.n_err() == 0 && res.n_fail() == 0;
}

}

