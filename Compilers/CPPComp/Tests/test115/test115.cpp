// A loop of seven `else if` (#54). Its chain ends on one .asm line that
// holds seven labels and the jump back; the passes read a line as at most 8
// words, so the jump's target was lost and the stack depths fell back to 128
// words each. hardware.txt checks the depths the processor gets now, 2 and
// 2; the outputs are what C++ gives for the input 1..8 (input_0.txt).

int main()
{
    int x;
    for (int k = 0; k < 8; k++) {
        x = in(0);
        if (x == 1) out(0, 10);
        else if (x == 2) out(0, 20);
        else if (x == 3) out(0, 30);
        else if (x == 4) out(0, 40);
        else if (x == 5) out(0, 50);
        else if (x == 6) out(0, 60);
        else if (x == 7) out(0, 70);
        else out(0, 0);
    }
    return 0;
}
