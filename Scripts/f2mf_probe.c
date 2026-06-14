// Harness de verificação do f2mf/mf2f do asmcomp (Compilers/ASMComp/Sources/t2t.c).
// Copia LITERAL das funções para reproduzir, fora da toolchain, o round-trip
// float -> "my float" -> float e medir o erro. Objetivo: confirmar o bug de
// overflow de arredondamento (mantissa toda-1s que soma 1 e estoura sem
// renormalizar -> decodifica como 0).
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// parâmetros do alvo (config padrão 32/23/8)
int nbmant = 23, nbexpo = 8, nubits = 32;

// ---- cópia literal de f2mf (t2t.c) -----------------------------------------
unsigned int f2mf(char *va, float *delta)
{
    float f = atof(va);
    if (f == 0.0) return 1 << (nbmant + nbexpo -1);
    int *ifl = (int*)&f;
    int s =  (*ifl >> 31) & 0x00000001;
    int e = ((*ifl >> 23) & 0xFF) - 127 - 22;
    int m = ((*ifl & 0x007FFFFF) + 0x00800000) >> 1;
    s = s << (nbmant + nbexpo);
    e = e + (23-nbmant);
    int sh = 0;
    while (e < -pow(2, nbexpo-1)) { e = e+1; sh = sh+1; }
    if (nbmant == 23) { if (*ifl & 0x00000001) m = m+1; }
    else { sh = 23-nbmant+sh; int carry = (m >> (sh-1)) & 1; m = m >> sh; if (carry) m = m+1; }
    if (m >> nbmant) { m = m >> 1; e = e + 1; } // FIX: renormalize mantissa overflow
    float num = (atof(va)<0.0) ? -atof(va) : atof(va);
    if (delta) *delta = m*pow(2,e)-num;
    e = e & ((int)(pow(2,nbexpo)-1));
    e = e << nbmant;
    return s + e + m;
}

// ---- itob: uint -> string binária MSB-first de `bits` bits ------------------
void itob(unsigned int v, int bits, char *out) {
    for (int i = 0; i < bits; i++) out[i] = ((v >> (bits-1-i)) & 1) ? '1' : '0';
    out[bits] = 0;
}

// ---- cópia literal de mf2f (t2t.c) -----------------------------------------
float mf2f(char *ifl)
{
    int s = ifl[0] == '1';
    char exb[64]; for (int i=0;i<nbexpo;i++) exb[i] = ifl[i+1]; exb[nbexpo]=0;
    int es = exb[0] == '1';
    if (es) for (int i=0;i<nbexpo;i++) exb[i] = (exb[i] == '1') ? '0' : '1';
    char *endp;
    int e = strtol(exb,&endp,2);
    if (es) e = -(e+1);
    char mab[64]; for (int i=0;i<nbmant;i++) mab[i] = ifl[nbexpo+1+i]; mab[nbmant]=0;
    int  m = strtol(mab,&endp,2);
    float  f = m * pow(2,e);
    if (s) f = -f;
    return f;
}

void test(const char *lit) {
    char buf[64]; strcpy(buf, lit);
    float in = atof(lit);
    float delta;
    unsigned int code = f2mf(buf, &delta);
    char bits[64]; itob(code, nubits, bits);
    float out = mf2f(bits);
    double relerr = (in != 0.0) ? fabs((double)(out-in)/in) : fabs(out-in);
    const char *flag = (relerr > 1e-3) ? "  <<<<< ERRO GROSSO" : "";
    printf("  in=%-26s  bits=%s  out=%-18g  relerr=%.3g%s\n", lit, bits, out, relerr, flag);
}

int main(void) {
    printf("Config: nubits=%d nbmant=%d nbexpo=%d\n\n", nubits, nbmant, nbexpo);
    printf("Round-trip  float -> my-float -> float :\n");
    test("0.0");
    test("1.0");
    test("1.5");
    test("2.0");
    test("3.14159265");
    test("100.0");
    // candidatos ao bug de overflow de arredondamento: mantissa IEEE = 0x7FFFFF
    // -> o maior float < proxima potencia de 2, com bit0 da mantissa setado.
    test("1.99999988079071044921875");   // (2 - 2^-23) * 2^0  = mantissa 0x7FFFFF
    test("3.99999976158142089843750");   // (2 - 2^-23) * 2^1
    test("0.99999994039535522460937500"); // (2 - 2^-23) * 2^-1
    return 0;
}
