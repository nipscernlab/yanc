// toma/cade fixture, project pass LINK3 (docs/toma-and-cade-links.md): the
// middle of a chain in C++, as lk_chain_b. Takes a word from link 0 (same
// clock: cade_sync) and hands twice it over link 1 (another clock: plain toma).
#pragma yanc nuioin 1
#pragma yanc nuioou 1

void main(void)
{
    while (1) {
        int x = cade_sync(0);
        toma(1, x * 2);
    }
}
