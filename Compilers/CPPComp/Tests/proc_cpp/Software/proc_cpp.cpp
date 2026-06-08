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

    out(0, sum);          // 10
    out(0, (int)prod);    // 3  (out() sends an integer word; cast the float --
                          //     prod itself still shows as 3.75 in GTKWave)
    out(0, data[3]);      // 13
}
