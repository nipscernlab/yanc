// #pragma once: mutually-#include'ing headers (a<->b) and the diamond they form
// resolve to a single definition of each function — no duplicate-symbol breakage
// and no infinite include recursion.
#pragma yanc prname tst24
#pragma yanc nubits 16

#include "t24_a.h"
#include "t24_b.h"          // second pull of both headers — both skipped by once

void main(void) {
    out(0, helper(10));     // 11
    out(0, dbl(6));         // 12
}
