// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include <typeinfo>
#include <iostream>
#include "tester.h"
using namespace std;

using namespace unitpp;

void tester::summary()
{
	os << "Tests [Ok-Fail-Error]: [" << n_test.n_ok() << '-'
	<< n_test.n_fail() << '-' << n_test.n_err() << "]\n";
}
void tester::visit(test& t)
{
	try {
		t();
		n_test.add_ok();
		write(t);
	} catch (assertion_error& e) {
		n_test.add_fail();
		write(t, e);
	} catch (exception& e) {
		n_test.add_err();
		write(t, e);
	} catch (...) {
		n_test.add_err();
		write(t, 0);
	}
}

void tester::visit(suite& t)
{
	if (verbose)
		os << "****** " << t.name() << " ******" << endl;
	accu.push(n_test);
}

void tester::visit(suite& , int)
{
	res_cnt r(accu.top());
	accu.pop();
	if (n_test.n_err() != r.n_err())
		n_suite.add_err();
	else if (n_test.n_fail() != r.n_fail())
		n_suite.add_fail();
	else
		n_suite.add_ok();
}
void tester::write(test& t)
{
	if (verbose)
		disp(t, "OK");
}
void tester::disp(test& t, const string& status)
{
	os << status << ": " << t.name() << endl;
}
void tester::write(test& t, assertion_error& e)
{
	disp(t, "FAIL");
	os << e << '\n';
}
void tester::write(test& t, std::exception& e)
{
	disp(t, "ERROR");
	os << "     : [" << typeid(e).name() << "] " << e.what() << '\n';
}
void tester::write(test& t, int )
{
	disp(t, "ERROR");
	os << "     : " << "unknown exception" << '\n';
}
