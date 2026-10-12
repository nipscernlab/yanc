#pragma yanc prname test65
// Bitfields laid out and masked at the 32-bit word, as a host compiler does.
// (1) Without `#pragma yanc nubits`, cppcomp packed bitfields into 16-bit
//     words: four 8-bit fields took two words, so a union with an `unsigned`
//     saw only the first two (raw = 513 instead of 67305985), and a 20-bit
//     field could not share a word with a 12-bit one.
// (2) A field as wide as the word got the mask (1L << 32) - 1, undefined with
//     Windows' 32-bit long and 0 in practice: `unsigned w : 32` always read 0.
struct Four { unsigned a : 8; unsigned b : 8; unsigned c : 8; unsigned d : 8; };
union  U    { unsigned raw; Four f; };
struct Mix  { unsigned lo : 20; unsigned hi : 12; };
union  MU   { unsigned raw; Mix bits; };
struct Wide { unsigned w : 32; };

void main(void) {
    U u;
    u.raw = 0;
    u.f.a = 1; u.f.b = 2; u.f.c = 3; u.f.d = 4;
    out(0, u.raw);                 // 67305985 = 0x04030201 (16-bit packing: 513)

    u.raw = 0x11223344;            // write the word, read the fields
    out(0, u.f.a);                 // 68 = 0x44
    out(0, u.f.b);                 // 51 = 0x33
    out(0, u.f.c);                 // 34 = 0x22
    out(0, u.f.d);                 // 17 = 0x11, the top byte

    u.f.d = 0x55;                  // write the top field, keep the others
    out(0, u.raw);                 // 1428304708 = 0x55223344

    MU v;
    v.raw = 0;
    v.bits.lo = 0xABCDE;           // 20 bits
    v.bits.hi = 0x123;             // 12 bits, same word
    out(0, v.raw);                 // 305839326 = 0x123ABCDE
    out(0, v.bits.lo);             // 703710
    out(0, v.bits.hi);             // 291

    Wide x;
    x.w = 123456;
    out(0, x.w);                   // 123456 (mask-0 bug: 0)
    x.w = x.w + 1;
    out(0, x.w);                   // 123457
}
