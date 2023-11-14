#include <gdk/gdkkeysyms.h>

#include <glibmm/refptr.h>

#include <gtkmm/builder.h>
#include <gtkmm/liststore.h>
#include <gtkmm/main.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/window.h>

#include "lazycellrenderer.hpp"

#define N_ROWS 250

using namespace Gtk;

Glib::RefPtr<Builder> builder;

void scroll_to_top() {
    ScrolledWindow* sw;
    builder->get_widget("sw_view", sw);
    auto adj = sw->get_vadjustment();
    adj->set_value(adj->get_lower());
}
void scroll_to_middle() {
    ScrolledWindow* sw;
    builder->get_widget("sw_view", sw);
    auto adj = sw->get_vadjustment();
    adj->set_value(
        ( adj->get_lower() + adj->get_upper() )/2
    );
}
void scroll_to_bottom() {
    ScrolledWindow* sw;
    builder->get_widget("sw_view", sw);
    auto adj = sw->get_vadjustment();
    adj->set_value(adj->get_upper());
}

gboolean on_key_press_event(GdkEventKey* key, Window* win) {
    switch (key->keyval) {
        case GDK_KEY_Escape:
            win->hide();
            return TRUE;
        default:
            return FALSE;
    }
}

int main() {
    auto app = Application::create();

    builder = Builder::create_from_file("ui.glade");

    //populate the model with some data
    auto model = Glib::RefPtr<ListStore>::cast_static(
        builder->get_object("model")
    );
    TreeModel::iterator row;
    for(int i = 0; i < N_ROWS; i++) {
        row = model->append();
        row->set_value(0, i);
    }

    LazyCellRenderer lazy_renderer {};
    TreeViewColumn col("Lazy", lazy_renderer);
    col.add_attribute(lazy_renderer, "id", 0);

    TreeView* view;
    builder->get_widget("view", view);
    view->append_column(col);

    Window* win = NULL;
    builder->get_widget("win", win);
    if (!win) {
        return 1;
    }
    win->signal_key_press_event().connect(
        [win] (GdkEventKey* key)->gboolean  {return on_key_press_event(key, win); }
    );

    Button* btn;
    builder->get_widget("btn_quit", btn);
    btn->signal_clicked().connect(
        [win] (void)->void  { win->hide(); }
    );
    builder->get_widget("btn_top", btn);
    btn->signal_clicked().connect( [] {scroll_to_top();} );
    builder->get_widget("btn_middle", btn);
    btn->signal_clicked().connect( [] {scroll_to_middle();} );
    builder->get_widget("btn_bottom", btn);
    btn->signal_clicked().connect( [] {scroll_to_bottom();} );

    win->present();
    win->show_all();
    return app->run(*win);
}
