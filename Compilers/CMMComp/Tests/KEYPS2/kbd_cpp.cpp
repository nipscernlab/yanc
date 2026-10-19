// The keyboard block (SAPHO/myKeyPS2.v) on a C++ SAPHO with an 8-bit word:
// a message from a string literal (8-bit pointers), then each key read with
// cade_sync(0) echoed to port 0. Built and run by the regress project pass
// KEYPS2 (TopLevel/keyps2_tb.v), ABNT2 layout.
#pragma yanc nubits 8
#pragma yanc nuioin 1
#pragma yanc nuioou 1

void escreve(const char *s)
{
    while (*s) out(0, *s++);
}

int main()
{
    escreve("ok:");
    while (1) out(0, cade_sync(0));
    return 0;
}
