#ifndef __UNITPP__GUITESTER_H
#define __UNITPP__GUITESTER_H
#ifdef GUI
#include "tester.h"
#include "gui.h"
#include <stack>
#include <vector>
#include <qobject.h>

/// \name unitpp
namespace unitpp {
/**
 * The class for setting up the GUI. It visites all the tests, and create
 * nodes for them in the test tree, count them and adjust all the GUI
 * elements appropriately.
 */
class g_setup : public QObject, public visitor {
	Q_OBJECT
private:
	gui* gp;
	bool running;
	int n_suites;
	int n_tests;
	std::stack<suite_node*> branch;
	std::vector<node*> nodes, selected;
	std::map<QListViewItem*, node*> rev;
	void find_selected(QListViewItem*);
	void add_node(node* np);
public:
	g_setup(gui* gp);
	virtual void visit(test&);
	virtual void visit(suite&);
	virtual void visit(suite&, int);
public slots:
	///
	void run();
};
}
#endif
#endif
