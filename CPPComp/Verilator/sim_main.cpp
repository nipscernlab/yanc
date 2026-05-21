// Generic Verilator harness for the YANC processor top module (compiled with
// --prefix Vtop, so the class is always Vtop regardless of the test's module
// name). Mirrors the auto-generated iverilog testbench protocol:
//   - req_in == 1  : processor is reading an input word; the bench presents the
//                    current input register on `in`, and on the negedge advances
//                    to the next value from the input file.
//   - out_en == 1  : processor is writing an output word; the bench records `out`
//                    on the posedge.
// Runs until `expected` outputs are captured (plus a short drain) or a cycle cap.
// Used by regress.sh for tests too heavy for iverilog (e.g. the FISTA port).
//
// Usage: sim <input.txt> <output.txt> <max_cycles> <expected_outputs>
#include "Vtop.h"
#include "verilated.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

double sc_time_stamp() { return 0; }   // required by verilated.cpp when present

int main(int argc, char** argv) {
    const char* in_path  = argc > 1 ? argv[1] : nullptr;
    const char* out_path = argc > 2 ? argv[2] : "output_0.txt";
    long max_cycles      = argc > 3 ? atol(argv[3]) : 200000000L;
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
    long drain = -1;        // cycles left to run after reaching `expected`
    long idle  = 0;         // cycles since the last output (program-finished detector)

    for (long cyc = 0; cyc < max_cycles; ++cyc) {
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

        if (drain > 0) { if (--drain == 0) break; }
        if (idle > 500000) break;     // no more output: the program has finished
    }

    FILE* of = fopen(out_path, "w");
    if (of) { for (int v : outputs) fprintf(of, "%d\n", v); fclose(of); }
    for (int v : outputs) printf("%d ", v);
    printf("\n");

    delete top;
    return 0;
}
