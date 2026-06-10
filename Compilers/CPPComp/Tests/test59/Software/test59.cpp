#pragma yanc prname test59
// Realistic embedded example: debounced digital input + rising-edge counter.
// debounce() keeps its state in function-local statics (the classic embedded
// idiom for per-function persistent state without globals or objects).
// A change of level is only accepted after 3 consecutive equal samples.

int debounce(int raw) {
    static int stable = 0;   // accepted (debounced) level
    static int cand   = 0;   // candidate level being confirmed
    static int run    = 0;   // consecutive samples agreeing with cand
    if (raw == stable) { run = 0; return stable; }
    if (raw == cand) {
        run = run + 1;
        if (run >= 3) { stable = cand; run = 0; }
    } else {
        cand = raw;
        run = 1;
    }
    return stable;
}

// Static-local counter: every call returns a fresh id.
int next_id() {
    static int id = 100;
    id = id + 1;
    return id;
}

void main(void) {
    // Noisy button stream: a 1-sample glitch (ignored), a real press
    // (3+ samples), a 2-sample bounce on release (ignored), real release.
    int stream[20] = { 0,0,1,0,0,  1,1,1,1,1,  0,0,1,1,0,  0,0,0,0,0 };

    int edges = 0;
    int prev = 0;
    for (int k = 0; k < 20; k = k + 1) {
        int lvl = debounce(stream[k]);
        if (lvl == 1 && prev == 0) edges = edges + 1;
        prev = lvl;
    }
    out(0, edges);       // expect 1 (one clean press; glitch and bounce ignored)
    out(0, prev);        // expect 0 (released at the end)

    out(0, next_id());   // expect 101
    out(0, next_id());   // expect 102
    out(0, next_id());   // expect 103
}
