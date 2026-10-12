// test52 - out-of-class destructor definition: `Class::~Class() { body }`.
// The class declares the destructor in-class (prototype only, no body) and
// the body is provided outside the class. A stack object's scope-exit must
// run that out-of-class destructor body. Locks the dtor_decl + out-of-class
// dtor_def grammar added for this feature.
#pragma yanc prname test52

class Beeper {
public:
    int v;
    ~Beeper();          // in-class destructor DECLARATION (prototype only)
};

Beeper::~Beeper() {     // out-of-class destructor DEFINITION (the feature)
    out(0, v);
}

void main(void) {
    {
        Beeper b;
        b.v = 42;
    }                   // b leaves scope here -> Beeper__dtor runs -> out 42
    out(0, 99);         // sentinel printed after the destructor
}
