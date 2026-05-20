// 3x3 matrix algebra (numerical / linear algebra). Exercises multidimensional
// arrays, nested-brace initializers, and 2D arrays passed as parameters.
#pragma yanc prname tst39
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

void matmul(int a[3][3], int b[3][3], int c[3][3]) {
    for (int i = 0; i < 3; i = i + 1)
        for (int j = 0; j < 3; j = j + 1) {
            int s = 0;
            for (int k = 0; k < 3; k = k + 1) s = s + a[i][k] * b[k][j];
            c[i][j] = s;
        }
}

int det3(int m[3][3]) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

void main(void) {
    int A[3][3] = { {1,2,3}, {4,5,6}, {7,8,10} };
    int I[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    int B[3][3] = { {2,0,0}, {0,2,0}, {0,0,2} };
    int C[3][3];

    matmul(A, I, C);            // C = A
    out(0, C[0][0]);            // 1
    out(0, C[1][2]);            // 6
    out(0, C[2][2]);            // 10

    matmul(A, B, C);            // C = 2A
    out(0, C[1][1]);            // 10

    out(0, det3(A));            // -3
    out(0, det3(I));            // 1
}
