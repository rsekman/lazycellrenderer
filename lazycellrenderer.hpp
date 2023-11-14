#include <chrono>
#include <iostream>
#include <fmt/core.h>
#include <memory>
#include <thread>

#include <glibmm/dispatcher.h>
#include <glibmm/objectbase.h>
#include <glibmm/property.h>

#include <gdkmm/rectangle.h>

#include <gtkmm/main.h>
#include <gtkmm/cellrenderertext.h>

#include "threadmap.hpp"
#include "threadq.hpp"

using namespace Gtk;
using namespace fmt;
using namespace std::chrono_literals;

#define SLEEP_TIME 250ms

typedef ::Cairo::RefPtr<::Cairo::Context> CtxPtr;

typedef struct cr_context_s {
    int id;
    const CtxPtr& cr;
    Widget& widget;
    const Gdk::Rectangle&   background_area;
    const Gdk::Rectangle&   cell_area;
    CellRendererState  	flags;
} cr_context;

class LazyCellRenderer : public CellRendererText {
    public:
        LazyCellRenderer() :
            Glib::ObjectBase(typeid(LazyCellRenderer)),
            CellRendererText(),
            property_id(*this, "id", 0)
        {
            std::cout << "Initialising lazy renderer" << std::endl;
            t = std::thread( [this] { worker_thread(); } );
        };
        void render_vfunc(
            const CtxPtr&           cr,
            Widget&                 widget,
            const Gdk::Rectangle&   background_area,
            const Gdk::Rectangle&   cell_area,
            CellRendererState  	    flags
        ) override {
            int id = property_id.get_value();
            cr_context ctx = {
                id, cr, widget, background_area, cell_area, flags
            };
            if (rendered.count(id)) {
                display_value(ctx);
            } else {
                render_queue.push(std::make_unique<cr_context>(ctx));
            }
        };
        ~LazyCellRenderer() {
            // ensure clean exit
            render_queue.close();
            t.join();
        }
    private :
        Glib::Property<int> property_id;
        ThreadMap<int, std::string> rendered;
        // ctx contains const refs, so we wrap it in a unique_ptr to pass it around
        ThreadQueue<std::unique_ptr<cr_context>> render_queue;
        std::thread t;
        Glib::Dispatcher dispatcher;

        void worker_thread() {
            std::cout << "Starting worker thread" << std::endl;
            std::optional<std::unique_ptr<cr_context>> ctx;

            while (
                render_queue.is_open()
                // queue closed => we are exiting => don't need to consume more values
                && (ctx = render_queue.pop()) != std::nullopt
            ) {
                if(rendered.count((*ctx)->id)) {
                    continue;
                }
                calculate_value((*ctx)->id);
                // Gtk functions must be called from the main thread
                dispatcher.connect(sigc::mem_fun((*ctx)->widget, &Widget::queue_draw));
                dispatcher.emit();
            }
        }

        void calculate_value(int id) {
            std::cout << format("Lazy rendering with id = {}... ", id);
            std::cout.flush();
            std::this_thread::sleep_for(SLEEP_TIME);
            rendered.insert_or_assign(id, format("This is row {}", id));
            std::cout << "done" << std::endl;
        }

        void display_value(const cr_context ctx) {
            property_text().set_value(rendered[ctx.id].c_str());
            CellRendererText::render_vfunc(
                ctx.cr,
                ctx.widget,
                ctx.background_area,
                ctx.cell_area,
                ctx.flags
            );
        }
};

