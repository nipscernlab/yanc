// Single inheritance (non-virtual): the base subobject is laid first, so a
// derived object inherits the base fields and methods.
#pragma yanc prname test9
class Animal {
public:
    int legs;
    int getLegs() { return legs; }
    int speak()   { return 1; }
};

class Dog : public Animal {
public:
    int tail;
    int total() { return legs + tail; }   // uses the inherited field
};

void main(void) {
    Dog d;
    d.legs = 4;                // inherited field
    d.tail = 1;
    out(0, d.legs);            // 4
    out(0, d.getLegs());       // 4  (inherited method)
    out(0, d.tail);            // 1
    out(0, d.total());         // 5  (own method, inherited field)
    out(0, d.speak());         // 1  (inherited method)
}
