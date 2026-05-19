// string literals as values (materialised char arrays, decay to char*)
#pragma yanc prname tst9
#pragma yanc nubits 16

char *msg = "AB";          // global char* initialised from a string literal

void main(void) {
    char *s = "Hi!";
    out(0, s[0]);          // 'H' = 72
    out(0, s[1]);          // 'i' = 105
    out(0, s[2]);          // '!' = 33
    out(0, msg[0]);        // 'A' = 65
    out(0, msg[1]);        // 'B' = 66

    // walk to the NUL terminator
    int i = 0;
    while (s[i]) {         // stops at the 0 byte
        out(0, s[i] + 1);  // 73 106 34
        i++;
    }
}
