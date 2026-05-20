// Hybrid recursion, harder cases: mutual recursion (a call-graph cycle that
// spans two functions, so the transitive-closure detector must flag both), a
// doubly-recursive move counter, and an in-place quicksort whose SCALAR locals
// (i, j, pivot, tmp) must each get their own per-level frame slot.
#pragma yanc prname tst60
#pragma yanc sdepth 128
#pragma yanc ndstac 128

// mutual recursion -> cycle spanning is_odd <-> is_even
int is_even(int n);
int is_odd(int n)  { if (n == 0) return 0; return is_even(n - 1); }
int is_even(int n) { if (n == 0) return 1; return is_odd(n - 1); }

// Towers of Hanoi move count, doubly recursive: 2^n - 1
int hanoi(int n) {
    if (n == 0) return 0;
    return hanoi(n - 1) + 1 + hanoi(n - 1);
}

// in-place quicksort over a global array, recursing on index bounds; the
// partition's working scalars live in the recursive frame
int arr[8] = { 5, 2, 8, 1, 9, 3, 7, 4 };

void qsort_rec(int lo, int hi) {
    int i, j, pivot, tmp;
    if (lo >= hi) return;
    pivot = arr[hi];
    i = lo;
    for (j = lo; j < hi; j = j + 1) {
        if (arr[j] < pivot) {
            tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
            i = i + 1;
        }
    }
    tmp = arr[i]; arr[i] = arr[hi]; arr[hi] = tmp;
    qsort_rec(lo, i - 1);
    qsort_rec(i + 1, hi);
}

void main(void) {
    int k;
    out(0, is_even(10));   // 1
    out(0, is_odd(7));     // 1
    out(0, is_even(7));    // 0
    out(0, hanoi(5));      // 31
    qsort_rec(0, 7);
    for (k = 0; k < 8; k = k + 1) out(0, arr[k]);  // 1 2 3 4 5 7 8 9
}
