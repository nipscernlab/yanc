// toma/cade fixture, project pass LINK (docs/design/toma-and-cade.md): the C++ side
// of a mixed pair. Takes the 16 values the C+- link_envia hands over and puts
// each on port 0, waiting less every turn.
#pragma yanc nuioin 1
#pragma yanc nuioou 1

void main(void)
{
    for (int n = 0; n < 16; n++) {
        for (volatile int k = 0; k < 16 - n; k++) { }
        int v = cade(0);
        out(0, v);
    }
}
