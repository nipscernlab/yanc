#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8
#pragma yanc nuioin 1
#pragma yanc nuioou 1

// The pragmas set the processor, as the header of my_program.cmm does: word
// width, float format, number of input and output ports. Its name comes
// from cppcomp -n.

int main() {
    int acc = 0;
    for (int k = 1; k <= 10; ++k) acc += k;
    out(0, acc);            // writes 55 to output port 0
    return 0;
}
