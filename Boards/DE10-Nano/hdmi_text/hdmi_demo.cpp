// The demonstration of SAPHO/myHDMIText.v on the DE10-Nano: a C++ SAPHO of
// 8 bits writes to the HDMI screen on link 1 with toma_sync, the same words a
// program writes to myVGAText (docs/peripherals.md). The screen is a test
// card for the picture: the title on the first line, the 95 printable
// characters, a line of 80 '=' that must touch both edges, a counter that
// changes every 60 ms (simulated at 25 MHz), and a line at the bottom.
// Built and simulated by the regress project pass HDMITEXT (Tests/HDMITEXT).
#pragma yanc nubits 8

void escreve(const char *s)
{
    while (*s) toma_sync(1, *s++);
}

void cursor(int linha, int coluna)
{
    toma_sync(1, -127);
    toma_sync(1, linha);
    toma_sync(1, coluna);
}

void espera()                                // 60 ms at 25 MHz (simulated)
{
    for (int a = 0; a < 100; a++)
        for (int b = 0; b < 100; b++)
            for (int c = 0; c < 20; c++) {}
}

int main()
{
    toma_sync(1, -128);
    escreve("SAPHO + myHDMIText na DE10-Nano: 80 x 30 caracteres, 640 x 480 a 60 Hz");
    cursor(2, 0);
    for (int c = 32; c < 127; c++) toma_sync(1, c);
    cursor(5, 0);
    for (int c = 0; c < 80; c++) toma_sync(1, '=');
    cursor(29, 0);
    escreve("linha 30 de 30");
    cursor(7, 0);
    escreve("contador: ");
    int d2 = '0', d1 = '0', d0 = '0';
    while (1) {
        cursor(7, 10);
        toma_sync(1, d2);
        toma_sync(1, d1);
        toma_sync(1, d0);
        espera();
        d0++;
        if (d0 > '9') { d0 = '0'; d1++; }
        if (d1 > '9') { d1 = '0'; d2++; }
        if (d2 > '9') d2 = '0';
    }
    return 0;
}
