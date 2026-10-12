// ****************************************************************************
// proc_cpp - minimal C++ program for the single_proc_cpp pipeline demo.
//
// The simplest possible processor: a handful of int and float variables
// (global and local) so we can watch how cppcomp logs them in cmm_log.txt --
// int -> type 1, float -> type 2 -- the metadata asmcomp later uses to mirror
// each variable into the VCD at its data-memory address. main() is not
// recursive, so every local lives at a FIXED data-memory address (it is not a
// stack-frame slot), which is exactly what the dump's `if (mem_addr_wr == X)`
// in proc.v needs to match.
// ****************************************************************************
#pragma yanc prname proc_cpp

int   g_count = 7;      // global int   -> cmm_log: global g_count 1
float g_gain  = 2.5;    // global float -> cmm_log: global g_gain  2

struct Point { int x; float y; };   // aggregate type (used below)

void main(void)
{
    int   a = 3;                // main a 1
    float b = 1.5;              // main b 2

    int   sum  = a + g_count;   // main sum  1   -> 10
    float prod = b * g_gain;    // main prod 2   -> 3.75

    // fixed-size arrays in a non-recursive function -> fixed data-memory
    // addresses, so each element gets its own GTKWave mirror (data0000.. /
    // coef0000..). cmm_log: "main data 1 4" and "main coef 2 3".
    int   data[4];
    float coef[3];

    data[0] = a;                // 3
    data[1] = sum;              // 10
    data[2] = g_count;          // 7
    data[3] = sum + a;          // 13

    coef[0] = b;                // 1.5
    coef[1] = prod;             // 3.75
    coef[2] = g_gain;           // 2.5

    // --- "complicated" data the dump must IGNORE (no GTKWave mirror) --------
    // A pointer and a reference hold an ADDRESS, and a struct is a multi-word
    // aggregate -> none of p / r / pt (nor pt.x / pt.y) get a cmm_log entry
    // (type-code 0 = not published). They still WORK; they just must not show
    // up as waveform variables. Only `target`, a plain int, is mirrored.
    int   target = 0;
    int*  p = &target;          // pointer   -> address value, not published
    int&  r = target;           // reference -> address under the hood, not published
    *p = 42;                    // write through the pointer
    r  = r + 1;                 //   ... and the reference -> target = 43

    Point pt;                   // struct aggregate -> not published
    pt.x = a;                   // 3
    pt.y = prod;                // 3.75

    out(0, sum);          // 10
    out(0, (int)prod);    // 3  (out() sends an integer word; cast the float --
                          //     prod itself still shows as 3.75 in GTKWave)
    out(0, data[3]);      // 13
    out(0, target);       // 43  (pointer + reference worked; neither is mirrored)
    out(0, pt.x);         // 3   (struct field worked; pt has no mirror)
}
