// test55 - early-exit RAII: an early return / break / continue must run the
// destructors of the class-typed locals it leaves scope of, in reverse
// construction order, BEFORE jumping. The destructor here prints the object id,
// so the output order is the proof. Also checks the returned value survives the
// destructor calls (which clobber the accumulator).
#pragma yanc prname test55

class Beep {
public:
    int id;
    Beep(int x) { id = x; }
    ~Beep();
};

Beep::~Beep() { out(0, id); }     // dtor prints the id

int f(int early) {
    Beep a(1);
    if (early) {
        Beep b(2);
        return 10;                // dtors b(2) then a(1); returns 10
    }
    Beep c(3);
    return 20;                    // dtors c(3) then a(1); returns 20
}

void g() {
    int i = 0;
    while (i < 3) {
        Beep d(50 + i);
        i = i + 1;
        if (i == 2) continue;     // dtor d(51) before re-test
        if (i == 3) break;        // dtor d(52) before exit
    }                             // normal iteration: dtor d(50) at block exit
}

void main(void) {
    int r1 = f(1);                // -> 2 1
    out(0, r1);                   // 10
    int r2 = f(0);                // -> 3 1
    out(0, r2);                   // 20
    g();                          // -> 50 51 52
    out(0, 999);
}
