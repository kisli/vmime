// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#ifdef GUI
#include "gui.h"
#include "unit++.h"
using namespace std;
using namespace unitpp;

gui_hook ghook;	// ensure linkage of the gui stuff

namespace {
// The test suite for the gui components
class Test : public suite
{
	void dummy()
	{
	}
public:
	Test() : suite("GUI test suite")
	{
		suite::main().add("gui", this);
		add("dummy", testcase(this, "Test nothing", &Test::dummy));
	}
} * theTest = new Test();

}
#endif
