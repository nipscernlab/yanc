// Static data member: one shared global per class (Class::count), used by the
// constructor and a method; instance ids come from the shared counter.
#pragma yanc prname test17
class Widget {
public:
    static int count = 0;     // one shared global across all instances
    int id;
    Widget() { count = count + 1; id = count; }   // ctor uses the static
    int total() { return count; }                 // method reads the static
};

void main(void) {
    out(0, Widget::count);   // 0  (qualified static access)
    Widget* a = new Widget;
    Widget* b = new Widget;
    out(0, Widget::count);   // 2
    out(0, a->id);           // 1
    out(0, b->id);           // 2
    out(0, a->total());      // 2  (shared static via a method)
}
