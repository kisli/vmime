// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#ifndef _UNITPP_TESTER_H
#define _UNITPP_TESTER_H
#include <stack>
#ifdef __UNITPP
#include "unit++.h"
#else
#include <unit++/unit++.h>
#endif
namespace unitpp {
/// A mostly internal class for keeping score.
class res_cnt {
	int ok, fail, err;
public:
	/// Create a 0 count.
	res_cnt() : ok(0), fail(0), err(0) {}
	/// Count one ok.
	void add_ok() { ++ok; }
	/// Count one fail.
	void add_fail() { ++fail; }
	/// Count one error.
	void add_err() { ++err; }
	/// get ok count.
	int n_ok() { return ok; }
	/// get fail count.
	int n_fail() { return fail; }
	/// get error count.
	int n_err() { return err; }
	/// get total count.
	int n() { return ok+fail+err; }
};
/**
 * The standard text based tester. It implements the visitor pattern for the
 * test and suite classes, and executes each test case in a depth first
 * traversal, toting the score of test cases.
 *
 * The class might be used for test executers aimed at other environments,
 * e.g. a GUI based version.
 *
 * Please note that this class is automagically instantiated by the main
 * method supplied in the library. This means that you might very well do all
 * your testing without directly laying hans on this fellow.
 */
class tester : public visitor {
	std::ostream& os;
	bool verbose; // list succeded tests
	std::stack<res_cnt> accu;
	res_cnt n_suite, n_test;
	void disp(test& t, const std::string&);
	void write(test& t);
	void write(test& t, assertion_error& e);
	void write(test& t, std::exception& e);
	void write(test& t, int dummy);
public:
	/**
	 * Create a text tester.
	 * \param os the stream to write results to.
	 * \param verbose whether to report on succesful tests
	 */
	tester(std::ostream& os, bool verbose = false) : os(os), verbose(verbose) {}
	/// Get the score for tests
	res_cnt res_tests() { return n_test; }
	/// Get the score for suites
	res_cnt res_suites() { return n_suite; }
	/// Write the summary
	virtual void summary();
	/// Part of the visitor pattern.
	virtual void visit(test&);
	/// Part of the visitor pattern.
	virtual void visit(suite&);
	/// Part of the visitor pattern; visit to suite after children.
	virtual void visit(suite& t, int);
};

}
#endif
