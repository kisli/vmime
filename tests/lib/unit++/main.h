// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include <iostream>
#include "tester.h"
#include "optmap.h"

/**
 * The main of a test program that executes the main test suite and then
 * reports the summary.
 *
 * A #-v# or #--verbose# will turn on verbose, that reports succesful test
 * cases; the default behaviour is to report only those that fails.
 */
int main(int argc, const char* argv[]);

/// @name{unitpp}
namespace unitpp {

/**
 * The verbose flag, in case somebody wants to piggyback it with more
 * meaning.
 */
extern bool verbose;

/**
 * A runner is the base class for the objects that actually processes the
 * tests from main. Main simply invokes the run_tests method of the current
 * test runner.
 * \Ref{main}
 */
class test_runner {
public:
	virtual ~test_runner();
	/**
	 * run all the tests with arguments in the argc, argv set
	 */
	virtual bool run_tests(int argc, const char** argv) = 0;
};
/**
 * Sets the test_runner to be used in testing. This hook allows another
 * tester to hook into the main function and replace the traditional tester.
 */
void set_tester(test_runner*);

/// A plain test runner for the ordinary text version.
class plain_runner : public test_runner {
public:
	/// Run the tests specified in argv, starting at i.
	virtual bool run_tests(int argc, const char** argv);
private:
/**
 * Run a test found in the suite::main() test by id. If id is empty run the
 * main test.
 * @name run_test-id
 * @return true, if the test was totally succesful.
 */
bool run_test(const std::string& id = "");
/// Run the test and return true if succesful. @see{run_test-id}
bool run_test(test*);
/// find the test with the given id
test* find_test(const std::string& id);
};
}
