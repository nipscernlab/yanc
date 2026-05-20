// Text codecs (strings / parsing). Run-length encoding and Roman-numeral
// parsing. Exercises string literals as char*, char arrays, pointers, and the
// subtractive-notation rule.
#pragma yanc prname tst41
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

int rle_encode(char *in, int n, char *out_ch, int *out_cnt) {
    int j = 0, i = 0;
    while (i < n) {
        char c = in[i];
        int run = 0;
        while (i < n && in[i] == c) { run = run + 1; i = i + 1; }
        out_ch[j] = c;
        out_cnt[j] = run;
        j = j + 1;
    }
    return j;   // number of runs
}

int roman_val(char c) {
    if (c == 'I') return 1;
    if (c == 'V') return 5;
    if (c == 'X') return 10;
    if (c == 'L') return 50;
    if (c == 'C') return 100;
    if (c == 'D') return 500;
    if (c == 'M') return 1000;
    return 0;
}

int roman_parse(char *s) {
    int total = 0, i = 0;
    while (s[i] != 0) {
        int v = roman_val(s[i]);
        int nx = roman_val(s[i + 1]);     // 0 at the terminator
        if (v < nx) total = total - v;
        else        total = total + v;
        i = i + 1;
    }
    return total;
}

void main(void) {
    char rch[16];
    int  rcnt[16];
    int runs = rle_encode("aaaabbbcdddd", 12, rch, rcnt);
    out(0, runs);            // 4
    out(0, (int)rch[0]);     // 'a' = 97
    out(0, rcnt[0]);         // 4 (aaaa)
    out(0, rcnt[1]);         // 3 (bbb)
    out(0, rcnt[3]);         // 4 (dddd)

    out(0, roman_parse("XIV"));       // 14
    out(0, roman_parse("MCMXCIV"));   // 1994
    out(0, roman_parse("MMXXIII"));   // 2023
}
