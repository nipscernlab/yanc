#pragma yanc prname test30
// Realistic embedded example: a device mode controller as an OO state machine
// (State pattern). States are polymorphic objects on the heap; a factory builds
// the current state, and transitions delete the old one and build the next.

class State {
public:
    virtual int next(int ev) = 0;   // next-state id for an event
    virtual int code() = 0;         // this state's reported code
    virtual ~State() {}
};

class Idle : public State {
public:
    int next(int ev) override { return ev == 1 ? 1 : 0; }   // start -> Running
    int code() override { return 10; }
};

class Running : public State {
public:
    int next(int ev) override { return ev == 2 ? 2 : 1; }   // fault -> Fault
    int code() override { return 20; }
};

class Fault : public State {
public:
    int next(int ev) override { return ev == 9 ? 0 : 2; }   // reset -> Idle
    int code() override { return 99; }
};

State* make(int id) {
    if (id == 0) return new Idle();
    if (id == 1) return new Running();
    return new Fault();
}

void main(void) {
    int events[5];
    events[0] = 1;   // start
    events[1] = 0;   // nothing
    events[2] = 2;   // fault
    events[3] = 9;   // reset
    events[4] = 0;   // nothing

    int cur = 0;
    State* s = make(cur);
    out(0, s->code());                 // 10 (Idle)

    for (int i = 0; i < 5; i = i + 1) {
        int nx = s->next(events[i]);
        if (nx != cur) {
            delete s;
            cur = nx;
            s = make(cur);
        }
        out(0, s->code());             // 20 20 99 10 10
    }
    delete s;
}
