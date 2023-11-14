# LazyCellRenderer

This repo contains a toy example of a lazy cell renderer for `GtkTreeView`.

## The problem

`GtkTreeView` and `GtkListStore` (or `GtkTreeStore`) are a convenient way to implement the model-view-controller pattern in Gtk.
However, the built-in views ("cell renderers") are basically just read operations.
They render a value already present in the model;
e.g. `CellRendererText` renders a string already present in the model.
But what if the view we want is an *expensive function* (in time and/or space) of the model values, and only a small fraction out of potentially thousands of rows are visible at a time?
One use case could be that one column contains pointers to images on disk, and we want to draw those images to `PixBuf`s.
The obvious solution is *lazy loading*:
only calculate `f(x)` for those `x` that are visible.
But to do this we must implement a custom cell renderer.

## Implementation

In this toy example the model contains a single column of type `int`.
The "expensive" function is just a formatted string, but we sleep for a noticable time to simulate an operation that requires more computation, fetches a resource from disk, etc.

We subclass `CellRendererText` to `LazyCellRenderer`.
This class contains
* A new Glib property `id` that will be set from the model
* A queue of values to be processed
* A hash map for memoisation
* A worker thread that consumes values from the queue and stores the results in the hash map
Both the queue and the hash map must be thread-safe: the queue (hash map) is written to (read) by the Gtk main thread and consumed (written to) by the worker thread.
The standard library's queue and hash map are wrapped together with a mutex to achieve this.
(Only the operations needed are implemented.)

The actual rendering is done in the virtual method `render_vfunc`.
We override this in `LazyCellRenderer`:
* If the computed value exists in the hash map, set the `text` property and call `render_vfunc` on the parent class.
* Otherwise, enqueue `id` for the worker thread to consume.
Once the computation finishes, Gtk must be told to redraw the view.
This must be done via a `Glib::Dispatcher`, since it's not safe to call widget methods from outside the Gtk main thread.

## Demo and building

The cell renderer is demonstrated with a small program that populates the model with many rows.
The user can watch the computed values being filled in successively.
Trace output in the console shows that only those values that would be visible are computed.
Buttons for scrolling to the top/middle/bottom of the view can be used to check.
Press `ESC` to exit.

Build with meson:
```sh
meson build
cd build && ninja && ./demo
```

## Extensions

Obviously, the cell renderer can be made more flexible by letting `calculate_value` be a function object provided by the user.
It could be implemented as an interface that can be added to each of the Gtk-provided cell renderers (Pixbuf, Progress, Spinner, Text, Accel, Combo, Spin, Toggle).
Finally, if the computed value uses a lot of memory, it would be prudent to use a smarter caching strategy than simply memoising every value.
