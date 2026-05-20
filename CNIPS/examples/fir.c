// FIR filter — implementation (multi-file example: source)
#include "fir.h"

void fir_init(struct fir *f, float *coefs) {
    f->pos = 0;
    for (int i = 0; i < FIR_N; i = i + 1) {
        f->coef[i] = coefs[i];
        f->buf[i]  = 0.0;
    }
}

float fir_step(struct fir *f, float x) {
    f->buf[f->pos] = x;
    float acc = 0.0;
    for (int i = 0; i < FIR_N; i = i + 1) {
        int idx = (f->pos - i + FIR_N) % FIR_N;   // tap i = sample i steps ago
        acc = acc + f->coef[i] * f->buf[idx];
    }
    f->pos = (f->pos + 1) % FIR_N;
    return acc;
}
