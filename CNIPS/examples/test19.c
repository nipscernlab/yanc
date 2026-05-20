// goto: backward (loop) and forward jumps. Exercises labels that the codegen
// emits with a leading underscore (@_l_func_label) — only resolvable after the
// assembler lexer was fixed to accept a leading underscore in labels.
#pragma yanc prname tst19
#pragma yanc nubits 16

void main(void) {
    int i = 0;
    int sum = 0;
loop:
    sum = sum + i;
    i = i + 1;
    if (i < 5) goto loop;      // backward goto: 0+1+2+3+4

    out(0, sum);               // 10

    if (sum == 10) goto ok;    // forward goto
    out(0, 999);               // skipped
ok:
    out(0, 7);                 // 7
}
