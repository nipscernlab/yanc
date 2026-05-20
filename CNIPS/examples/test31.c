// Conway's Game of Life (simulation / 2D grids). Exercises multidimensional
// arrays, nested loops, continue, &&/||, function calls, object-like macros.
#pragma yanc prname tst31
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

#define H 6
#define W 6

int grid[H][W];
int nxt[H][W];

int neighbors(int r, int c) {
    int n = 0;
    for (int dr = -1; dr <= 1; dr = dr + 1)
        for (int dc = -1; dc <= 1; dc = dc + 1) {
            if (dr == 0 && dc == 0) continue;
            int rr = r + dr;
            int cc = c + dc;
            if (rr >= 0 && rr < H && cc >= 0 && cc < W)
                n = n + grid[rr][cc];
        }
    return n;
}

void step(void) {
    for (int r = 0; r < H; r = r + 1)
        for (int c = 0; c < W; c = c + 1) {
            int n = neighbors(r, c);
            nxt[r][c] = (grid[r][c] && (n == 2 || n == 3)) || (!grid[r][c] && n == 3);
        }
    for (int r = 0; r < H; r = r + 1)
        for (int c = 0; c < W; c = c + 1) grid[r][c] = nxt[r][c];
}

int count(void) {
    int s = 0;
    for (int r = 0; r < H; r = r + 1)
        for (int c = 0; c < W; c = c + 1) s = s + grid[r][c];
    return s;
}

void main(void) {
    // a "blinker": three cells in a row — oscillates with period 2
    grid[2][1] = 1; grid[2][2] = 1; grid[2][3] = 1;

    out(0, count());     // 3
    out(0, grid[1][2]);  // 0

    step();              // becomes vertical
    out(0, count());     // 3
    out(0, grid[2][2]);  // 1 (center persists)
    out(0, grid[1][2]);  // 1
    out(0, grid[2][1]);  // 0

    step();              // back to horizontal
    out(0, grid[2][1]);  // 1
    out(0, grid[1][2]);  // 0
}
