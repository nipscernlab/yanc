// Demonstra que a suíte de regressão é CEGA ao bug de overflow do f2mf:
// roda o codificador BUGADO e o CORRIGIDO sobre uma lista de floats (stdin)
// e imprime só os casos onde eles divergem (= o bug dispararia naquele valor).
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int nbmant = 23, nbexpo = 8, nubits = 32;

static unsigned int enc(char *va, int fix) {
    float f = atof(va);
    if (f == 0.0) return 1 << (nbmant + nbexpo - 1);
    int *ifl = (int*)&f;
    int s =  (*ifl >> 31) & 1;
    int e = ((*ifl >> 23) & 0xFF) - 127 - 22;
    int m = ((*ifl & 0x007FFFFF) + 0x00800000) >> 1;
    s = s << (nbmant + nbexpo);
    e = e + (23 - nbmant);
    int sh = 0;
    while (e < -pow(2, nbexpo-1)) { e++; sh++; }
    if (nbmant == 23) { if (*ifl & 1) m = m + 1; }
    else { sh = 23-nbmant+sh; int c = (m >> (sh-1)) & 1; m = m >> sh; if (c) m++; }
    if (fix) { if (m >> nbmant) { m = m >> 1; e = e + 1; } } // <- a correção
    e = e & ((int)(pow(2,nbexpo)-1));
    e = e << nbmant;
    return s + e + m;
}
static float dec(unsigned int code) {
    int s = (code >> (nbmant+nbexpo)) & 1;
    int ef = (code >> nbmant) & ((1<<nbexpo)-1);
    int es = (ef >> (nbexpo-1)) & 1;
    int e = es ? (ef - (1<<nbexpo)) : ef;          // dois-complementos do expoente
    int m = code & ((1<<nbmant)-1);
    float f = m * pow(2,e);
    return s ? -f : f;
}

int main(void) {
    char line[256];
    int n = 0, hits = 0;
    while (fgets(line, sizeof line, stdin)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0]) continue;
        float in = atof(line);
        if (in == 0.0f) continue;
        n++;
        float vb = dec(enc(line, 0));   // bugado
        float vf = dec(enc(line, 1));   // corrigido
        double eb = fabs((double)(vb - in)/in);
        double ef = fabs((double)(vf - in)/in);
        if (eb > 1e-3 && ef <= 1e-3) { // bug dispara aqui (bugado erra grosso, corrigido acerta)
            hits++;
            printf("  OVERFLOW: %-28s  bugado=%-14g  corrigido=%-14g\n", line, vb, vf);
        }
        if (ef > 5e-3) { // erro grosso AINDA na versao corrigida (denormal/outro)
            printf("  GROSSO(fix): %-24s  in=%-16g  corrigido=%-14g  relerr=%.3g\n", line, in, vf, ef);
        }
    }
    printf("\nLiterais float testados do corpus: %d\n", n);
    printf("Quantos disparam o bug de overflow: %d\n", hits);
    return 0;
}
