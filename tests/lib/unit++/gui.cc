#include <algorithm>
#include "gui.h"
#include "unit++.h"
#ifdef HAVE_SSTREAM
#include <sstream>
#endif
using namespace unitpp;
using namespace std;

#ifdef GUI
static const char* const img_error[] = { 
"14 14 2 1",
". c #ff0000",
"# c #ffffff",
"...########...",
"....######....",
".....####.....",
"#.....##.....#",
"##..........##",
"###........###",
"####......####",
"####......####",
"###........###",
"##..........##",
"#.....##.....#",
".....####.....",
"....######....",
"...########..."};

static const char* const img_fail[] = { 
"14 14 2 1",
"# c #ff0000",
". c #ffffff",
"..............",
".##........##.",
".###......###.",
"..###....###..",
"...###..###...",
"....######....",
".....####.....",
".....####.....",
"....######....",
"...###..###...",
"..###....###..",
".###......###.",
".##........##.",
".............."};

static const char* const img_ok[] = { 
"14 14 3 1",
"# c #00ff00",
"a c #00ff6a",
". c #ffffff",
"..........###.",
".........###..",
"........#a#...",
"........aa#...",
".......#aa....",
".......#a.....",
"......#aa.....",
"#.....aa#.....",
"###..#a#......",
".##.##a.......",
"..#aaa#.......",
"...#aa#.......",
"....##........",
".....#........"};

static const char* const img_empty[] = { 
"14 14 2 1",
". c #f0f0f0",
"# c #ffffff",
"..............",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".############.",
".............."};


cnt_item::cnt_item(QWidget* par, const QString& txt, const QColor& col,
		const char* name)
	: QHBox(par, name), v(0)
{
	setSpacing(3);
	setMargin(5);
	val = new QLabel("0", this);
	val->setAlignment(QLabel::AlignVCenter | QLabel::AlignRight);
	QPalette pal(val->palette());
	QColorGroup cg(pal.active());
	cg.setColor(QColorGroup::Foreground, col);
	pal.setActive(cg);
	val->setPalette(pal);
	label = new QLabel(txt, this);
}
void cnt_item::value(int iv)
{
	v = iv;
	val->setNum(v);
}
void cnt_item::inc()
{
	value(v+1);
}

cnt_line::cnt_line(const QString& txt, QWidget* par, const char* name)
	: QHBox(par, name)
{
	label = new QLabel(txt, this);
	QFont font(label->font());
	font.setBold(true);
	label->setFont(font);
	cnts[0] = new cnt_item(this, "Total");
	cnts[1] = new cnt_item(this, "OK", green);
	cnts[2] = new cnt_item(this, "Fail", red);
	cnts[3] = new cnt_item(this, "Error", red);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}

void cnt_line::max(int m) { cnts[id_max]->value(m); }
void cnt_line::inc_ok() { cnts[id_ok]->inc(); }
void cnt_line::inc_fail() { cnts[id_fail]->inc(); }
void cnt_line::inc_error() { cnts[id_error]->inc(); }
void cnt_line::reset()
{
	for (int i = id_ok; i < n_id; ++i)
		cnts[i]->value(0);
}

res_stack::res_stack(const QString& txt, QWidget* par, const char* name)
	: QVBox(par, name)
{
	cnts = new cnt_line(txt, this, name);
	bar = new QProgressBar(this);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}
void res_stack::max(int v)
{
	cnts->max(v);
	bar->setTotalSteps(v);
}
void res_stack::inc_progress(bool err)
{
	QPalette pal(bar->palette());
	QColorGroup cg(pal.active());
	QColor red(255,0,0);
	QColor green(0,244,0);
	cg.setColor(QColorGroup::Highlight, err ? red : green);
	cg.setColor(QColorGroup::HighlightedText, black);
	pal.setActive(cg);
	pal.setInactive(cg);
	pal.setDisabled(cg);
	bar->setPalette(pal);
	bar->setProgress(bar->progress()+1);
}
void res_stack::reset() {
	cnts->reset();
	bar->setProgress(0);
}
void res_stack::inc_ok()    { cnts->inc_ok(); inc_progress(false); }
void res_stack::inc_fail()  { cnts->inc_fail(); inc_progress(true); }
void res_stack::inc_error() { cnts->inc_error(); inc_progress(true); }

QHBox* behave(QHBox* box, bool x_expand, bool y_expand)
{
	box->setSizePolicy(QSizePolicy(
				x_expand ? QSizePolicy::Preferred : QSizePolicy::Maximum,
				y_expand ? QSizePolicy::Preferred : QSizePolicy::Maximum));
	box->setFrameShape(QFrame::StyledPanel);
	box->setFrameShadow(QFrame::Raised);
	box->setMargin(11);
	box->setSpacing(6);
	return box;
}

void node::setImg()
{
	static QPixmap i_emp((const char**)img_empty);
	static QPixmap i_ok((const char**)img_ok);
	static QPixmap i_fail((const char**)img_fail);
	static QPixmap i_err((const char**)img_error);
	static QPixmap* imgs[] = { &i_emp, &i_ok, &i_fail, &i_err };
	item->setPixmap(0, *(imgs[st]));
	if (st > is_ok)
		for (QListViewItem* ip = item; ip != 0; ip = ip->parent())
			ip->setOpen(true);
};

node::node(suite_node* par, test& t)
	: item(new QListViewItem(par->lvi(), t.name().c_str())), t(t), st(none)
{
	par->add_child(this);
	setImg();
}
node::node(gui* gp, test& t)
	: item(new QListViewItem(gp->test_tree(), t.name().c_str())), t(t), st(none)
{
	setImg();
	item->setOpen(true);
}
void node::run()
{
	for (QListViewItem* ip=item->firstChild(); ip != 0; ip=item->firstChild())
		delete ip;
	try {
		t();
		status(is_ok);
		emit ok();
	} catch (assertion_error& e) {
		status(is_fail);
		show_error(e);
		emit fail();
	} catch (exception& e) {
		status(is_error);
		show_error(e.what());
		emit error();
	} catch (...) {
		status(is_error);
		show_error("unknown ... exception");
		emit error();
	}
}
void node::show_error(assertion_error& e)
{
#ifdef HAVE_SSTREAM
	ostringstream oss;
	oss << e;
	show_error(oss.str().c_str());
#else
	show_error(e.what());	// not well, but some sign
#endif
}
void node::show_error(const char* msg)
{
	QListViewItem* elvi = new QListViewItem(item, msg);
	elvi->setSelectable(false);
}
suite_node::suite_node(suite_node* par, suite& t) : node(par, t) { }
suite_node::suite_node(gui* par, suite& t) : node(par, t) { }
void suite_node::run()
{
	status(is_ok);
	for (cctyp::iterator p = cc.begin(); p != cc.end(); ++p) {
		(*p)->run();
		status(max(status(), (*p)->status()));
	}
	switch (status()) {
	case is_ok: emit ok(); break;
	case is_fail: emit fail(); break;
	case is_error: emit error(); break;
	}
}
gui::gui(QApplication& app, QWidget* par, const char* name)
	: QVBox(par, name), app(app)
{
	setMargin(6);
	setSpacing(3);
	tree = new QListView(behave(new QVBox(this), true, true), "tree");
	tree->addColumn("Test");
	tree->setFrameShadow(QListView::Sunken);
	tree->setResizePolicy(QScrollView::Manual);
	tree->setRootIsDecorated(true);
	QVBox* f_cnts = new QVBox(this);
	behave(f_cnts, true, false);
	suites = new res_stack("Suites", f_cnts);
	tests = new res_stack("Tests", f_cnts);
	QHBox* hbox = behave(new QHBox(this), true, false);
	b_run = new QPushButton("Run", hbox, "run");
	b_stop = new QPushButton("Stop", hbox, "stop");
	b_quit = new QPushButton("Quit", hbox, "quit");
	connect(b_quit, SIGNAL(clicked()), &app, SLOT(quit()));
	connect(b_run, SIGNAL(clicked()), this, SLOT(run_pressed()));
	connect(b_stop, SIGNAL(clicked()), this, SLOT(stop_pressed()));
}

gui::~gui() { }

void gui::processEvents(int t)
{
	app.processEvents(t);
}
void gui::reset()
{
	tests->reset();
	suites->reset();
}
void gui::nconnect(node* n, res_stack* rs)
{
	connect(n, SIGNAL(ok()), rs, SLOT(inc_ok()));
	connect(n, SIGNAL(fail()), rs, SLOT(inc_fail()));
	connect(n, SIGNAL(error()), rs, SLOT(inc_error()));
}
void gui::add_test(node* n)
{
	nconnect(n, tests);
}
void gui::add_suite(node* n)
{
	nconnect(n, suites);
}

void gui::totSuites(int v) { suites->max(v); }
void gui::totTests(int v) { tests->max(v); }

#endif
