#pragma yanc prname test24
// Realistic embedded example: a tiny cooperative task scheduler.
// Tasks are plain function-pointer callbacks held in a fixed array; run()
// dispatches them in order. Classic bare-metal cooperative scheduler shape.
// Exercises: function-pointer typedef, empty () parameter lists, an array of
// callbacks as a class member, and indirect dispatch tasks[i]().

typedef void (*Task)();

int g_counter;

void taskA() { g_counter = g_counter + 1; }
void taskB() { g_counter = g_counter + 10; }
void taskC() { g_counter = g_counter * 2; }

class Scheduler {
    Task tasks[4];
    int n;
public:
    Scheduler() : n(0) {}
    void add(Task t) { tasks[n] = t; n = n + 1; }
    void run() {
        for (int i = 0; i < n; i = i + 1) tasks[i]();
    }
};

void main(void) {
    g_counter = 0;
    Scheduler s;
    s.add(taskA);    // +1  -> 1
    s.add(taskB);    // +10 -> 11
    s.add(taskC);    // *2  -> 22
    s.add(taskA);    // +1  -> 23
    s.run();
    out(0, g_counter);   // 23
}
