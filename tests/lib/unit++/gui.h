#ifndef __UNITPP_GUI_H
#define __UNITPP_GUI_H
#ifdef GUI
#include "tester.h"
#include <exception>
#include <vector>
#include <qwidget.h>
#include <qcolor.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qprogressbar.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qapplication.h>

/// \name unitpp
namespace unitpp {
/// A colored count with a unit.
class cnt_item : public QHBox
{
	Q_OBJECT
private:
	int v;
	QLabel* val;
	QLabel* label;
public:
	cnt_item(QWidget* par, const QString& txt, const QColor& col = black,
			const char* nam = 0);
public slots:
	void value(int v);
	void inc();
};

/// A line with total, ok, fail, and error counts.
class cnt_line : public QHBox
{
	Q_OBJECT
private:
	enum fields { id_max, id_ok, id_fail, id_error, n_id };
	QLabel* label;
	cnt_item* cnts[n_id];
public slots:
	void max(int v);
	void reset();
	void inc_ok();
	void inc_fail();
	void inc_error();
public:
	cnt_line(const QString& txt, QWidget* par = 0, const char* name = 0);
};

/// A cnt_line stacked with a progress bar.
class res_stack : public QVBox
{
	Q_OBJECT
private:
	cnt_line* cnts;
	QProgressBar* bar;
	void inc_progress(bool red);
public slots:
	void max(int max);
	void reset();
	void inc_ok();
	void inc_fail();
	void inc_error();
public:
	res_stack(const QString& txt, QWidget* par=0, const char* name=0);
};
class node;
/// The whole GUI box with test tree, results, and buttons.
class gui : public QVBox
{
	Q_OBJECT
public:
	gui(QApplication& app, QWidget* par = 0, const char* name = 0);
	virtual ~gui();
	QListView* test_tree() { return tree; }
	void add_test(node* n);
	void add_suite(node* n);
	void processEvents(int t);
signals:
	void run();
	void stop();
public slots:
	void totSuites(int v);
	void totTests(int v);
	void reset();
private slots:
	void run_pressed() { emit run(); }
	void stop_pressed() { emit stop(); }
private:
	void nconnect(node* node, res_stack*);
	QApplication& app;
	QListView* tree;
	res_stack* suites;
	res_stack* tests;
	QPushButton* b_run;
	QPushButton* b_stop;
	QPushButton* b_quit;
};
class suite_node;
// a Qt error prevents this from being a ListViewItem...
/**
 * A node in the test tree. An error in Qt prevents this to be derived from
 * QListViewItem, hence the separation.
 */
class node : public QObject
{
	Q_OBJECT
public:
	enum state { none, is_ok, is_fail, is_error };
	/// Create this node under par.
	node(suite_node* par, test&);
	/// Get the associated QListViewItem.
	QListViewItem* lvi() { return item; }
	///
	state status() { return st; }
signals:
	/// [signal] emitted when the test succedes
	void ok();
	/// [signal] emitted when the test fails
	void fail();
	/// [signal] emitted when the test throws an exception
	void error();
public slots:
	/// [slot] Make the test run, and emit appropriate signals.
	virtual void run();
protected:
	/// Make a top level test, directly under the gui.
	node(gui* par, test&);
	/// Set the status of the node, including update of the displayed icon.
	void status(state s) {
		st = s;
		setImg();
	}
private:
	void show_error(assertion_error& e);
	void show_error(const char*);
	QListViewItem* item;
	test& t;
	state st;
	void setImg();
};
/**
 * A specialized node representing a test suite.
 */
class suite_node : public node
{
	typedef std::vector<node*> cctyp;
	cctyp cc; // child container
public:
	/// Inner suite creation.
	suite_node(suite_node* par, suite&);
	/// Top level suite_node.
	suite_node(gui* par, suite&);
	/// Test.
	virtual void run();
	/// Register a node below this.
	void add_child(node* n) { cc.push_back(n); }
};
}
#endif
#endif
