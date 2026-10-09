// toma/cade fixture, project pass LINK: the C++ side of a mixed pair, one link
// both ways. Hands n to the C+- link_pong, takes back n + 1, puts it on port 0.
#pragma yanc prname link_cpp_ping
#pragma yanc nuioin 1
#pragma yanc nuioou 1

void main(void)
{
    for (int n = 0; n < 16; n++) {
        toma(n);
        out(0, cade());     // cade() as an argument
    }
}
