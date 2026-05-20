// XTEA block cipher (cryptography). 64-bit block as two 32-bit words; exercises
// 32-bit unsigned arithmetic, logical shifts, XOR, array parameters, and large
// hex constants (0x9E3779B9 needs the full 32-bit literal). Tests encrypt then
// decrypt == original (round-trip), and records the ciphertext.
#pragma yanc prname tst32
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

void xtea_encrypt(unsigned int v[2], unsigned int key[4]) {
    unsigned int v0 = v[0], v1 = v[1], sum = 0;
    unsigned int delta = 0x9E3779B9;
    for (int i = 0; i < 32; i = i + 1) {
        v0 = v0 + ((((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]));
        sum = sum + delta;
        v1 = v1 + ((((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]));
    }
    v[0] = v0; v[1] = v1;
}

void xtea_decrypt(unsigned int v[2], unsigned int key[4]) {
    unsigned int v0 = v[0], v1 = v[1];
    unsigned int delta = 0x9E3779B9;
    unsigned int sum = delta * 32;             // delta << 5
    for (int i = 0; i < 32; i = i + 1) {
        v1 = v1 - ((((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]));
        sum = sum - delta;
        v0 = v0 - ((((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]));
    }
    v[0] = v0; v[1] = v1;
}

void main(void) {
    unsigned int key[4] = { 0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F };
    unsigned int p0 = 0x01234567, p1 = 0x89ABCDEF;
    unsigned int v[2];
    v[0] = p0; v[1] = p1;

    xtea_encrypt(v, key);
    out(0, (int)v[0]);                 // ciphertext word 0 (deterministic)
    out(0, (int)v[1]);                 // ciphertext word 1

    xtea_decrypt(v, key);
    out(0, (v[0] == p0) && (v[1] == p1)); // 1 — round-trip recovered the plaintext
}
