// 8-point DFT (signal processing). Exercises IEEE-754 float complex arithmetic,
// float lookup tables (nested initializers), and pointer parameters. Verified
// against known transforms: a DC signal, an impulse, and a pure cosine.
#pragma yanc prname tst44
// default 32-bit / IEEE-754 single

float cos8[8] = {  1.0,  0.70710678,  0.0, -0.70710678, -1.0, -0.70710678,  0.0,  0.70710678 };
float sin8[8] = {  0.0,  0.70710678,  1.0,  0.70710678,  0.0, -0.70710678, -1.0, -0.70710678 };

void dft8(float *xr, float *xi, float *Xr, float *Xi) {
    for (int k = 0; k < 8; k = k + 1) {
        float sr = 0.0, si = 0.0;
        for (int n = 0; n < 8; n = n + 1) {
            int m = (k * n) % 8;
            float wr = cos8[m];
            float wi = -sin8[m];                 // e^{-i 2pi kn/8}
            sr = sr + (xr[n] * wr - xi[n] * wi);
            si = si + (xr[n] * wi + xi[n] * wr);
        }
        Xr[k] = sr;
        Xi[k] = si;
    }
}

void main(void) {
    float zero[8] = { 0,0,0,0,0,0,0,0 };
    float Xr[8], Xi[8];

    float dc[8] = { 1,1,1,1,1,1,1,1 };           // constant -> all energy in bin 0
    dft8(dc, zero, Xr, Xi);
    out(0, (int)Xr[0]);     // 8
    out(0, (int)Xr[1]);     // 0
    out(0, (int)Xr[4]);     // 0

    float imp[8] = { 1,0,0,0,0,0,0,0 };          // impulse -> flat spectrum (1 each)
    dft8(imp, zero, Xr, Xi);
    out(0, (int)Xr[0]);     // 1
    out(0, (int)Xr[3]);     // 1

    float cosw[8] = { 1.0, 0.70710678, 0.0, -0.70710678, -1.0, -0.70710678, 0.0, 0.70710678 };
    dft8(cosw, zero, Xr, Xi);   // cos at bin 1 -> peaks at bins 1 and 7 (=4)
    out(0, (int)Xr[1]);     // 4
    out(0, (int)Xr[7]);     // 4
}
