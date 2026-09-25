#pragma yanc prname test63
// Regression for the f2mf rounding-carry bug: the largest float just below a
// power of two has an all-ones IEEE mantissa, so rounding to the YANC mantissa
// carries out of the field (0x7FFFFF -> 0x800000). Without renormalization that
// carry bled into the exponent and the value encoded as 0.0 — so e.g.
// 1.99999988f (= 2 - 2^-23) silently became 0. Each constant below sits on a
// power-of-two boundary; multiplied by 1000 and truncated it must NOT be 0.
// Since the exact encoder (yanc_num, TODO 5) each decimal is rounded ONCE,
// straight to the 23-bit SAPHO mantissa -- not first to a 24-bit IEEE float
// and then again. Next to 2 the SAPHO step is 2^-22, so 1.99999988 lies
// between 2 - 2^-22 = 1.99999976 (1.18e-7 away) and 2.0 (1.20e-7 away): the
// nearest is 2 - 2^-22, and x1000 truncates to 1999. b and d likewise.
void main(void) {
    float a = 1.99999988f;   // -> 2 - 2^-22 (nearest)          1999.99976
    float b = 0.99999994f;   // -> 1 - 2^-23 (nearest)           999.99988
    float c = 3.9999998f;    // -> 4.0       (nearest)
    float d = 0.49999997f;   // -> 0.5 - 2^-24 (nearest)         499.99994
    float e = 1.999999f;     // farther below 2 (control)
    out(0, (int)(a * 1000.0f));   // 1999
    out(0, (int)(b * 1000.0f));   //  999
    out(0, (int)(c * 1000.0f));   // 4000
    out(0, (int)(d * 1000.0f));   //  499
    out(0, (int)(e * 1000.0f));   // 1999
}
