#pragma yanc prname test63
// Regression for the f2mf rounding-carry bug: the largest float just below a
// power of two has an all-ones IEEE mantissa, so rounding to the YANC mantissa
// carries out of the field (0x7FFFFF -> 0x800000). Without renormalization that
// carry bled into the exponent and the value encoded as 0.0 — so e.g.
// 1.99999988f (= 2 - 2^-23) silently became 0. Each constant below sits on a
// power-of-two boundary; multiplied by 1000 and truncated it must NOT be 0.
void main(void) {
    float a = 1.99999988f;   // 2 - 2^-23   -> rounds to 2.0
    float b = 0.99999994f;   // 1 - 2^-24   -> rounds to 1.0
    float c = 3.9999998f;    // ~4          -> rounds to 4.0
    float d = 0.49999997f;   // ~0.5        -> rounds to 0.5
    float e = 1.999999f;     // farther below 2 (control, already worked)
    out(0, (int)(a * 1000.0f));   // 2000
    out(0, (int)(b * 1000.0f));   // 1000
    out(0, (int)(c * 1000.0f));   // 4000
    out(0, (int)(d * 1000.0f));   //  500
    out(0, (int)(e * 1000.0f));   // 1999
}
