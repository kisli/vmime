// Copyright (C) 2001 Claus Dræby
// Terms of use are in the file COPYING
#include <algorithm>
#include <functional>
#include "unit++.h"

using namespace unitpp;
using namespace std;

#ifndef GUI
gui_hook::gui_hook() { }
#endif
void test::visit(visitor* v)
{
	v->visit(*this);
}
testcase::testcase(test* t)
: cnt(new size_t(1)), tst(t)
{
}
testcase::testcase(const testcase& t)
: cnt(t.cnt), tst(t.tst)
{
	(*cnt)++;
}
void testcase::dec_cnt()
{
	if (--(*cnt) == 0) {
		delete cnt;
		delete tst;
	}
}
testcase::~testcase()
{
	dec_cnt();
}
testcase& testcase::operator=(const testcase& t)
{
	++*(t.cnt);
	dec_cnt();
	cnt = t.cnt;
	tst = t.tst;
	return *this;
}
suite& suite::main()
{
	static suite instance("top");
	return instance;
}
test* suite::get_child(const string& id)
{
	vector<string>::iterator p = std::find(ids.begin(), ids.end(), id);
	if (p != ids.end())
		return &(static_cast<test&>(tests[p - ids.begin()]));
	return 0;
}
vector<string> unitpp::vectorize(const string& str, char c)
{
	vector<string> res;
	string::const_iterator s, p;
	for (s = str.begin(); s != str.end(); ) {
		p = find(s, str.end(), c);
		res.push_back(string(s, p));
		s = (p == str.end()) ? p : p + 1;
	}
	return res;
}
test* suite::find(const string& id)
{
	vector<string> ss(vectorize(id, '.'));
	test* tp = this;
	for (vector<string>::iterator p = ss.begin(); p != ss.end(); ++p)
		if (!(tp = tp->get_child(*p)))
			break;
	return tp;
}
void suite::add(const string& id, const testcase& t)
{
	ids.push_back(id);
	tests.push_back(t);
}
void suite::visit(visitor* v)
{
	v->visit(*this);
	for_each(tests.begin(), tests.end(),
		bind2nd(mem_fun_ref(&testcase::visit), v));
	v->visit(*this, 0);
}

void unitpp::assertion_error::out(ostream& os) const
{
	os << msg << string(" [assertion failed]");
}
