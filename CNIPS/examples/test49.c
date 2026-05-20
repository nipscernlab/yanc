// Base64 encoding (data encoding). Exercises an indexed lookup table, byte-wise
// bit manipulation (6-bit groups), ternary padding, and array parameters.
#pragma yanc prname tst49

char *B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int b64_encode(int *in, int n, char *out) {
    int o = 0;
    for (int i = 0; i < n; i = i + 3) {
        int b0 = in[i];
        int b1 = (i + 1 < n) ? in[i + 1] : 0;
        int b2 = (i + 2 < n) ? in[i + 2] : 0;
        out[o] = B64[b0 >> 2];                          o = o + 1;
        out[o] = B64[((b0 & 3) << 4) | (b1 >> 4)];      o = o + 1;
        out[o] = (i + 1 < n) ? B64[((b1 & 15) << 2) | (b2 >> 6)] : '=';  o = o + 1;
        out[o] = (i + 2 < n) ? B64[b2 & 63] : '=';      o = o + 1;
    }
    return o;
}

void main(void) {
    char out[16];

    int man[3] = { 'M', 'a', 'n' };       // "Man" -> "TWFu"
    int n = b64_encode(man, 3, out);
    out(0, n);              // 4
    out(0, (int)out[0]);    // 'T' = 84
    out(0, (int)out[1]);    // 'W' = 87
    out(0, (int)out[2]);    // 'F' = 70
    out(0, (int)out[3]);    // 'u' = 117

    int m[1] = { 'M' };                   // "M" -> "TQ=="
    b64_encode(m, 1, out);
    out(0, (int)out[0]);    // 'T' = 84
    out(0, (int)out[1]);    // 'Q' = 81
    out(0, (int)out[2]);    // '=' = 61
    out(0, (int)out[3]);    // '=' = 61
}
