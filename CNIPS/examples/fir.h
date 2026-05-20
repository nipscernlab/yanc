// FIR filter — public interface (multi-file example: header)
#pragma once
#define FIR_N 4

struct fir {
    float coef[FIR_N];   // tap coefficients
    float buf[FIR_N];    // circular delay line
    int   pos;
};

void  fir_init(struct fir *f, float *coefs);
float fir_step(struct fir *f, float x);
