// Verilator side of the width sweep: drives ops.v with the oracle's vectors and
// compares every output as a W-bit hex string. Port types change with W
// (CData/SData/IData/QData up to 64 bits, VlWide<N> above), so the get/set
// helpers are overloads on the port type and the harness never names it.
//   usage: Vops <vec.txt> <W>
#include "Vops.h"
#include "verilated.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

static const char* HEX = "0123456789abcdef";
double sc_time_stamp() { return 0; }   // required by verilated.cpp

// ---- set a port from a W-bit hex string ------------------------------------
template <typename T> static void set_int(T& p, const std::string& h) {   // T = uint8/16/32/64
    uint64_t v = 0; for (char ch : h) v = (v << 4) | (uint64_t)(std::strchr(HEX, ch) - HEX);
    p = (T)v;
}
static void set(uint8_t&  p, const std::string& h) { set_int(p, h); }
static void set(uint16_t& p, const std::string& h) { set_int(p, h); }
static void set(uint32_t& p, const std::string& h) { set_int(p, h); }
static void set(uint64_t& p, const std::string& h) { set_int(p, h); }
template <size_t N> static void set(VlWide<N>& p, const std::string& h) {
    for (size_t w = 0; w < N; w++) p[w] = 0;
    // hex digits from the right, 8 per 32-bit word
    size_t d = h.size();
    for (size_t w = 0; w < N && d > 0; w++) {
        uint32_t v = 0; int shift = 0;
        for (int k = 0; k < 8 && d > 0; k++, shift += 4) { d--; v |= (uint32_t)(std::strchr(HEX, h[d]) - HEX) << shift; }
        p[w] = v;
    }
}

// ---- read a port as a W-bit hex string (exactly W/4 digits) ----------------
static std::string hex_of(uint64_t v, int digits) {
    std::string s(digits, '0');
    for (int i = digits - 1; i >= 0; i--) { s[i] = HEX[v & 15]; v >>= 4; }
    return s;
}
static std::string get(uint8_t  p, int dig) { return hex_of(p, dig); }
static std::string get(uint16_t p, int dig) { return hex_of(p, dig); }
static std::string get(uint32_t p, int dig) { return hex_of(p, dig); }
static std::string get(uint64_t p, int dig) { return hex_of(p, dig); }
template <size_t N> static std::string get(const VlWide<N>& p, int dig) {
    std::string s(dig, '0');
    int i = dig - 1;
    for (size_t w = 0; w < N && i >= 0; w++) { uint32_t v = p[w]; for (int k = 0; k < 8 && i >= 0; k++, i--) { s[i] = HEX[v & 15]; v >>= 4; } }
    return s;
}

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: Vops <vec.txt> <W>\n"); return 2; }
    Verilated::commandArgs(argc, argv);
    const int W = std::atoi(argv[2]);
    const int DIG = W / 4;
    std::ifstream in(argv[1]);
    if (!in) { std::fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }

    Vops* top = new Vops;
    const char* names[14] = {"add","sub","mul","div","mod","udiv","umod","shl","shr","sra","lt","gt","ult","eq"};
    long bad[14] = {0}; long n = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string a, b, e[14];
        if (!(ss >> a >> b)) continue;
        for (int i = 0; i < 14; i++) ss >> e[i];
        set(top->a, a); set(top->b, b);
        top->eval();
        std::string got[14] = {
            get(top->add, DIG), get(top->sub, DIG), get(top->mul, DIG), get(top->div, DIG), get(top->mod, DIG),
            get(top->udiv, DIG), get(top->umod, DIG), get(top->shl, DIG), get(top->shr, DIG), get(top->sra, DIG),
            hex_of(top->lt, 1), hex_of(top->gt, 1), hex_of(top->ult, 1), hex_of(top->eq, 1) };
        for (int i = 0; i < 14; i++) {
            if (got[i] != e[i]) {
                if (bad[i] == 0) std::printf("  vl %s: a=%s b=%s got=%s exp=%s\n", names[i], a.c_str(), b.c_str(), got[i].c_str(), e[i].c_str());
                bad[i]++;
            }
        }
        n++;
    }
    std::printf("VERILATOR W=%d vectors=%ld\n  ", W, n);
    for (int i = 0; i < 14; i++) std::printf("%s %ld ", names[i], bad[i]);
    std::printf("\n");
    top->final(); delete top;
    return 0;
}
