// The two peripherals on a C++ SAPHO with an 8-bit word: SAPHO/myKeyPS2.v on
// link 0 (cade_sync) and SAPHO/myVGAText.v on link 1 (toma_sync). A message
// from a string literal, a cursor move (command 129, line, column), then
// every key echoed to the screen. Built and run by the regress project pass
// VGATEXT (TopLevel/vgatext_tb.v).
#pragma yanc nubits 8

void escreve(const char *s)
{
    while (*s) toma_sync(1, *s++);
}

void cursor(int linha, int coluna)
{
    toma_sync(1, 129);
    toma_sync(1, linha);
    toma_sync(1, coluna);
}

int main()
{
    toma_sync(1, 128);
    escreve("SAPHO");
    cursor(2, 4);
    while (1) toma_sync(1, cade_sync(0));
    return 0;
}
