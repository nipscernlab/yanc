// inline assembly escape hatch: emit raw YANC instructions
#pragma yanc prname tst16
#pragma yanc nubits 16

void main(void) {
    int x;
    x = 10;

    // operands use the asm-level name (locals are <func>_<var>)
    asm("LOD main_x");      // acc = x
    asm("ADD main_x");      // acc = x + x
    asm("SET main_x");      // x = 20
    out(0, x);              // 20

    // multi-line string -> one instruction per line
    asm("LOD 5\nOUT 0");    // outputs 5

    // mix asm with normal C afterwards
    int y = x + 3;
    out(0, y);              // 23
}
