// Finite state machine via a function-pointer dispatch table (control systems).
// Exercises enums, a global array of function pointers with a nested
// initializer, and indirect calls through a table indexed by the current state.
#pragma yanc prname tst42
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

enum { GREEN, YELLOW, RED, NSTATES };

int reds = 0;

int on_green(void)  { return YELLOW; }
int on_yellow(void) { return RED; }
int on_red(void)    { reds = reds + 1; return GREEN; }

// table of state handlers, indexed by the current state
int (*handlers[NSTATES])(void) = { on_green, on_yellow, on_red };

void main(void) {
    int state = GREEN;
    for (int i = 0; i < 7; i = i + 1) {
        out(0, state);                 // 0 1 2 0 1 2 0
        state = handlers[state]();      // dispatch through the table
    }
    out(0, reds);                       // 2  (RED visited at steps 2 and 5)
}
