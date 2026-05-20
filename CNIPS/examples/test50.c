// Breadth-first shortest path through a maze (graph search). Exercises 2D
// arrays with nested initializers, an explicit queue (no recursion on this
// target), and neighbour iteration with bounds/wall checks.
#pragma yanc prname tst50
#define R 5
#define C 5

int maze[R][C] = {
    { 0, 0, 0, 1, 0 },
    { 1, 1, 0, 1, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
int dist[R][C];
int qr[64], qc[64];

void main(void) {
    for (int r = 0; r < R; r = r + 1)
        for (int c = 0; c < C; c = c + 1) dist[r][c] = -1;

    int dr[4] = { -1, 1, 0, 0 };
    int dc[4] = { 0, 0, -1, 1 };

    int head = 0, tail = 0;
    qr[tail] = 0; qc[tail] = 0; tail = tail + 1;
    dist[0][0] = 0;

    while (head < tail) {
        int r = qr[head];
        int c = qc[head];
        head = head + 1;
        for (int k = 0; k < 4; k = k + 1) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr >= 0 && nr < R && nc >= 0 && nc < C
                && maze[nr][nc] == 0 && dist[nr][nc] < 0) {
                dist[nr][nc] = dist[r][c] + 1;
                qr[tail] = nr; qc[tail] = nc; tail = tail + 1;
            }
        }
    }

    out(0, dist[4][4]);   // shortest path length 0,0 -> 4,4 = 8
    out(0, dist[2][2]);   // 4
    out(0, dist[0][4]);   // 8
    out(0, tail);         // reachable open cells = 18
}
