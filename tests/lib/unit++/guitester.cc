#ifdef GUI
#include "optmap.h"
#include "main.h"
#include "guitester.h"
#include <qapplication.h>

using namespace unitpp;
using namespace std;

class gui_test_runner : public test_runner {
	virtual bool run_tests(int argc, const char** argv)
	{
		QApplication a(argc, const_cast<char**>(argv));
		gui g(a);
		g_setup setup(&g);
		suite::main().visit(&setup);
		a.setMainWidget(&g);
		g.show();
		return a.exec();
	}
};
class gui_flag : public options_utils::optmap::cmd {
	bool do_cmd(options_utils::optmap* om)
	{
		static gui_test_runner gtr;
		set_tester(&gtr);
		return true;
	}
};
gui_hook::gui_hook()
{
	options().add("g", new gui_flag());
	options().alias("gui", "g");
}


g_setup::g_setup(gui* gp) : gp(gp), running(false), n_suites(0), n_tests(0)
{
	connect(gp, SIGNAL(run()), this, SLOT(run()));
}
void g_setup::add_node(node* np)
{
	nodes.push_back(np);
	rev[np->lvi()] = np;
}
void g_setup::visit(test& t)
{
	++n_tests;
	node* np = new node(branch.top(), t);
	add_node(np);
	gp->add_test(np);
}
void g_setup::visit(suite& t)
{
	++n_suites;
	suite_node* np = branch.size() ? new suite_node(branch.top(), t)
		: new suite_node(gp, t);
	branch.push(np);
	add_node(np);
	gp->add_suite(np);
}
void g_setup::visit(suite& t, int)
{
	branch.pop();
	if (!branch.size()) {
		gp->totSuites(n_suites);
		gp->totTests(n_tests);
	}
}

void g_setup::run()
{
	if (running)
		return;
	running = true;
	selected.clear();
	find_selected(gp->test_tree()->firstChild());
	if (!selected.size())
		selected.push_back(rev[gp->test_tree()->firstChild()]);
	gp->reset();
	for (vector<node*>::iterator p = selected.begin(); p!=selected.end(); ++p) {
		(*p)->run();
		gp->processEvents(20);
		if (!running)
			break;
	}
	running = false;
}

void g_setup::find_selected(QListViewItem* lvi)
{
	if (!lvi)
		return;
	if (lvi->isSelected())
		selected.push_back(rev[lvi]);
	else
		find_selected(lvi->firstChild());
	find_selected(lvi->nextSibling());
}
#endif
