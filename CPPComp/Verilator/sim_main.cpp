// Generic Verilator harness for the YANC processor top module (compiled with
// --prefix Vtop, so the class is always Vtop regardless of the test's module
// name). Mirrors the auto-generated iverilog testbench protocol:
//   - req_in == 1  : processor is reading an input word; the bench presents the
//                    current input register on `in`, and on the negedge advances
//                    to the next value from the input file.
//   - out_en == 1  : processor is writing an output word; the bench records `out`
//                    on the posedge.
// Termination order:
//   1. if Vtop exposes a `cheguei` pin (= the program has #TOAQUI), the harness
//      ends the simulation the cycle that pin goes high (after capturing any
//      output that posed in the same cycle).
//   2. otherwise, runs until `expected` outputs are captured plus a short drain.
//   3. otherwise, after IDLE_THRESHOLD cycles with no output (heuristic;
//      scales with max_cycles so heavy computations between out() calls
//      don't get mistaken for a finished program).
//   4. otherwise, the cycle cap.
// Used by regress.sh for tests too heavy for iverilog (e.g. the FISTA port).
//
// Usage: sim <input.txt> <output.txt> <max_cycles> <expected_outputs>
#include "Vtop.h"
#include "verilated.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

double sc_time_stamp() { return 0; }   // required by verilated.cpp when present

// SFINAE: read top->cheguei if Verilator exposed it (program has #TOAQUI),
// else return false. Lets the same harness drive any YANC program.
template <typename T>
auto read_cheguei(T* top, int) -> decltype(top->cheguei != 0) { return top->cheguei != 0; }
template <typename T>
bool read_cheguei(T*, ...) { return false; }

int main(int argc, char** argv) {
    const char* in_path  = argc > 1 ? argv[1] : nullptr;
    const char* out_path = argc > 2 ? argv[2] : "output_0.txt";
    // Use long long throughout: on Windows MSVC/MinGW `long` is 32-bit, so a
    // budget > 2.1G overflows and the simulation exits with no output. Cycle
    // budgets in the tens of G are now common (FISTA over a 1000-sample window).
    long long max_cycles = argc > 3 ? atoll(argv[3]) : 200000000LL;
    int  expected        = argc > 4 ? atoi(argv[4]) : -1;

    std::vector<int> inputs;
    if (in_path) {
        FILE* f = fopen(in_path, "r");
        if (f) { int v; while (fscanf(f, "%d", &v) == 1) inputs.push_back(v); fclose(f); }
    }

    Vtop* top = new Vtop;
    top->clk = 0; top->rst = 1; top->in = 0;
    top->eval();

    int  in_reg = 0;        // mirrors the bench `in_0` register (starts at 0)
    size_t idx  = 0;        // next unread input value
    std::vector<int> outputs;
    long long drain = -1;        // cycles left to run after reaching `expected`
    long long idle  = 0;         // cycles since the last output (program-finished detector)

    for (long long cyc = 0; cyc < max_cycles; ++cyc) {
        if (cyc == 1) top->rst = 0;   // release reset after the first cycle (rst high ~10ns)

        // ---- negedge clk ----
        top->clk = 0;
        if (top->req_in) top->in = in_reg;
        top->eval();
        if (top->req_in) {            // negedge $fscanf: advance to the next word
            in_reg = (idx < inputs.size()) ? inputs[idx++] : 0;
            top->in = in_reg;
            top->eval();
        }

        // ---- posedge clk ----
        top->clk = 1;
        if (top->req_in) top->in = in_reg;
        top->eval();
        if (top->out_en) {            // posedge write
            outputs.push_back((int)top->out);
            idle = 0;
            if (expected > 0 && (int)outputs.size() >= expected && drain < 0) drain = 50;
        } else if (!outputs.empty()) {
            ++idle;
        }

        // cheguei -> end-of-program marker (#TOAQUI). Capture this cycle's
        // output (above) then stop. SFINAE returns false when the program has
        // no #TOAQUI (the pin doesn't exist on Vtop).
        if (read_cheguei(top, 0)) break;

        if (drain > 0) { if (--drain == 0) break; }
        // idle threshold: scale with the cycle budget so heavy compute between
        // out() calls (e.g. quickselect/FISTA over a 1000-sample window can take
        // ~1G cycles) is not misread as "program finished". Floor at 500k for
        // backward compat with the original short tests.
        long long idle_threshold = max_cycles / 10;
        if (idle_threshold < 500000LL) idle_threshold = 500000LL;
        if (idle > idle_threshold) break;     // no more output: the program has finished
    }

    FILE* of = fopen(out_path, "w");
    if (of) { for (int v : outputs) fprintf(of, "%d\n", v); fclose(of); }
    for (int v : outputs) printf("%d ", v);
    printf("\n");

    delete top;
    return 0;
}
