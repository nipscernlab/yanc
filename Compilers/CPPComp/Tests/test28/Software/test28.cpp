#pragma yanc prname test28
// Realistic embedded example: a device status register accessed either as a
// whole word or field-by-field, via a union over a bitfield struct. Classic
// memory-mapped-register HAL idiom. Exercises a union used by its bare name
// (no `union` keyword) plus bitfield read/write through both views.

struct StatusBits {
    unsigned ready : 1;
    unsigned error : 1;
    unsigned mode  : 2;
    unsigned count : 4;
};

union Status {
    unsigned raw;
    StatusBits bits;
};

void main(void) {
    Status s;
    s.raw = 0;

    s.bits.ready = 1;
    s.bits.mode  = 2;          // binary 10
    s.bits.count = 5;          // binary 0101

    out(0, s.bits.ready);      // 1
    out(0, s.bits.mode);       // 2
    out(0, s.bits.count);      // 5

    // raw layout: ready(bit0)=1, error(bit1)=0, mode(bits2-3)=2<<2=8, count(bits4-7)=5<<4=80
    out(0, s.raw);             // 1 + 8 + 80 = 89

    // write through the raw word, read back as fields
    s.raw = 0xF3;              // 1111 0011
    out(0, s.bits.ready);      // 1   (bit0)
    out(0, s.bits.error);      // 1   (bit1)
    out(0, s.bits.mode);       // 0   (bits2-3 = 00)
    out(0, s.bits.count);      // 15  (bits4-7 = 1111)
}
