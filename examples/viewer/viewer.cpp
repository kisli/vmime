//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

//
// EXAMPLE DESCRIPTION:
// ====================
// A simple MIME viewer to show all the components of a message.
// The user interface is written using GTK+ 2.6.
//
// For more information, please visit:
// http://www.vmime.org/
//

#include <iostream>
#include <fstream>
#include <vector>

#include <gtk/gtk.h>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"



GtkWidget* window = NULL;
GtkWidget* treeView = NULL;
GtkWidget* textArea = NULL;

GtkTreeStore* treeModel = NULL;

vmime::ref <vmime::message> currentMessage = NULL;



void insertRowInModel(GtkTreeStore* model, vmime::ref <const vmime::component> comp, GtkTreeIter* parent = NULL)
{
	GtkTreeIter iter;

	gtk_tree_store_append(model, &iter, parent);
	gtk_tree_store_set(model, &iter, 0, typeid(*comp).name(), 1, comp.get(), -1);

	const std::vector <vmime::ref <const vmime::component> > children = comp->getChildComponents();

	for (int i = 0 ; i < children.size() ; ++i)
	{
		insertRowInModel(model, children[i], &iter);
	}
}


void updateTreeView()
{
	GtkTreeStore* model = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeView)));

	g_object_ref(model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), NULL);

	gtk_tree_store_clear(model);

	insertRowInModel(model, currentMessage);

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(model));
	g_object_unref(model);
}


static void treeViewSelChanged(GtkTreeView* treeView, gpointer userData)
{
	GtkTreePath* path = NULL;
	GtkTreeViewColumn* col = NULL;

	gtk_tree_view_get_cursor(treeView, &path, &col);

	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(treeModel), &iter, path);

	vmime::component* comp = NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(treeModel), &iter, 1, &comp, -1);

	GtkTextBuffer* textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textArea));
	GtkTextIter start, end;

	gtk_text_buffer_get_iter_at_offset(textBuffer, &start, comp->getParsedOffset());
	gtk_text_buffer_get_iter_at_offset(textBuffer, &end, comp->getParsedOffset() + comp->getParsedLength());

	gtk_text_buffer_select_range(textBuffer, &start, &end);

	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(textArea), &start, 0.0, FALSE, 0.0, 0.0);

	gtk_tree_path_free(path);
}


static void destroy(GtkWidget* widget, gpointer data)
{
	gtk_main_quit();
}


void openFile(const std::string& filename)
{
	std::ifstream file;
	file.open(filename.c_str(), std::ios::in | std::ios::binary);

	if (!file)
	{
		std::cerr << "Can't open file '" << filename << "'." << std::endl;
		return;
	}

	vmime::string data;
	char buffer[16384];

	do
	{
		file.read(buffer, sizeof(buffer));
		data += vmime::string(buffer, file.gcount());
	}
	while (file.gcount());

	vmime::ref <vmime::message> msg = vmime::create <vmime::message>();
	msg->parse(data);

	currentMessage = msg;

	char* convData = g_convert_with_fallback(data.c_str(), data.length(),
		"UTF-8", "ISO-8859-1", "?", NULL, NULL, NULL);

	if (convData == NULL)
	{
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textArea)),
			"GLib UTF-8 conversion error.", -1);
	}
	else
	{
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textArea)),
			convData, strlen(convData));

		g_free(convData);
	}

	updateTreeView();
}


static void onFileOpen()
{
	GtkWidget* dlg = gtk_file_chooser_dialog_new
		("Open Message File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
		 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		 NULL);

	if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
	{
		char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));

		openFile(filename);

		g_free(filename);
	}

	gtk_widget_destroy(dlg);
}



// UI definitions
static const GtkActionEntry uiActions[] =
{
	{ "FileMenu", NULL, "_File" },
	{ "FileOpen", GTK_STOCK_OPEN, "_Open...", "<control>O", NULL, G_CALLBACK(onFileOpen) },
	{ "FileExit", GTK_STOCK_QUIT, "_Exit", "<control>Q", NULL, G_CALLBACK(gtk_main_quit) }
};

static const char* uiDefinition =
	"<ui>" \
	"	<menubar name=\"MainMenuBar\">" \
	"		<menu action=\"FileMenu\">" \
	"			<menuitem action=\"FileOpen\"/>" \
	"			<menuitem action=\"FileExit\"/>" \
	"		</menu>" \
	"	</menubar>" \
	"</ui>";


int main(int argc, char* argv[])
{
	// VMime initialization
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

	// GTK+ initialization
	gtk_init(&argc, &argv);

	// Create a new window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_default_size(GTK_WINDOW(window), 800, 550);
	gtk_window_set_title(GTK_WINDOW(window), "VMime Viewer Example");

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	// Menubar
	GtkActionGroup* actionGroup = gtk_action_group_new ("Actions");
	gtk_action_group_add_actions(actionGroup, uiActions, G_N_ELEMENTS(uiActions), NULL);

	GtkUIManager* uiManager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(uiManager, actionGroup, 1);
	gtk_ui_manager_add_ui_from_string(uiManager, uiDefinition, -1, NULL);

	GtkWidget* menuBar = gtk_ui_manager_get_widget(uiManager, "/MainMenuBar");

	gtk_box_pack_start(GTK_BOX(vbox), menuBar, FALSE, FALSE, 0);

	// Horizontal Pane
	GtkWidget* hpane = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(vbox), hpane, TRUE, TRUE, 0);

	// Tree View
	treeModel = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

	treeView = gtk_tree_view_new();

	g_signal_connect(G_OBJECT(treeView), "cursor-changed", G_CALLBACK(treeViewSelChanged), NULL);

	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll), treeView);

	gtk_paned_add1(GTK_PANED(hpane), scroll);

	GtkTreeViewColumn* col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Component Name");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), col);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(treeModel));
	g_object_unref(treeModel);

	gtk_widget_set_size_request(treeView, 200, 100);

	// Text Area
	textArea = gtk_text_view_new();

	gtk_text_view_set_editable(GTK_TEXT_VIEW(textArea), FALSE);

	scroll = gtk_scrolled_window_new(NULL, NULL);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(scroll), textArea);

	gtk_paned_add2(GTK_PANED(hpane), scroll);

	// Show main window
	gtk_widget_show_all(window);

	// GTK main loop
	gtk_main();

	return 0;
}


