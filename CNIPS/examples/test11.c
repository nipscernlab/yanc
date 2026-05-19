// #if constant expressions + defined()
#pragma yanc prname tst11
#pragma yanc nubits 16

#define VER 3
#define DBG 0

void main(void) {
#if VER > 2
    out(0, 100);            // VER=3 > 2  -> yes
#else
    out(0, 200);
#endif

#if VER == 1
    out(0, 1);
#elif VER == 3
    out(0, 3);              // matches
#else
    out(0, 9);
#endif

#if defined(DBG) && DBG
    out(0, 999);            // DBG defined but 0 -> no
#else
    out(0, 5);             // yes
#endif

#if (VER * 10 + 1) == 31
    out(0, 31);            // 3*10+1 == 31 -> yes
#endif

#if !defined(NOPE)
    out(0, 7);             // NOPE undefined -> yes
#endif
}
