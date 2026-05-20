// N-Queens by backtracking recursion (combinatorial search). Places one queen
// per row; `place` recurses to depth N with a per-level loop counter living in
// its stack frame (stresses frame-local access under recursion). Occupancy is
// tracked in global arrays since a recursive function can't hold array locals.
// The leaf test `safe` is non-recursive, so it keeps fast fixed-address locals.
#pragma yanc prname tst61
#pragma yanc sdepth 128
#pragma yanc ndstac 128

#define N 8

int col[N];          // col[c]   = 1 when column c is occupied
int diag1[2 * N];    // "/" diagonals, indexed by r + c
int diag2[2 * N];    // "\" diagonals, indexed by r - c + N
int solutions;

int safe(int r, int c) {
    return !col[c] && !diag1[r + c] && !diag2[r - c + N];
}

void place(int r) {
    int c;
    if (r == N) { solutions = solutions + 1; return; }
    for (c = 0; c < N; c = c + 1) {
        if (safe(r, c)) {
            col[c] = 1; diag1[r + c] = 1; diag2[r - c + N] = 1;
            place(r + 1);
            col[c] = 0; diag1[r + c] = 0; diag2[r - c + N] = 0;
        }
    }
}

void main(void) {
    solutions = 0;
    place(0);
    out(0, solutions);   // 8 queens -> 92 distinct solutions
}
