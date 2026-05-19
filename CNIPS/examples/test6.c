// short-circuit && / || + ++/-- on complex lvalues
#pragma yanc prname tst6
#pragma yanc nubits 16

int mark(int r) { out(0, 555); return r; }   // side effect = prints 555

void main(void) {
    int a = 0;
    int b = 1;

    // && short-circuits: a==0 so mark() must NOT run
    if (a && mark(1)) out(0, 1); else out(0, 2);   // expect 2 (no 555)

    // || short-circuits: b!=0 so mark() must NOT run
    if (b || mark(1)) out(0, 3); else out(0, 4);   // expect 3 (no 555)

    // && evaluates RHS when LHS true: mark() runs
    if (b && mark(7)) out(0, 8);                    // expect 555 then 8

    // ++/-- on array element
    int arr[3];
    arr[0] = 10; arr[1] = 20; arr[2] = 30;
    arr[1]++;                   // 20 -> 21
    out(0, arr[1]);             // 21
    out(0, arr[1]++);           // post: prints 21, arr[1] -> 22
    out(0, arr[1]);             // 22

    // ++ on pointer (pointer arithmetic step = 1 word for int)
    int *p = arr;
    out(0, *p);                 // arr[0] = 10
    p++;                        // p -> arr[1]
    out(0, *p);                 // 22
}
