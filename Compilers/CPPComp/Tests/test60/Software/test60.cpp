#pragma yanc prname test60
// Realistic embedded example: sensor statistics passed around BY VALUE.
// A small POD struct is returned by value from the analysis function,
// passed by value into a scaling helper (caller's copy must NOT change),
// and copied by assignment (copies must be independent).

struct Stats {
    int min;
    int max;
    int mean;
};

Stats compute_stats(const int* v, int n) {
    Stats s;
    s.min = v[0];
    s.max = v[0];
    int sum = 0;
    for (int k = 0; k < n; k = k + 1) {
        if (v[k] < s.min) s.min = v[k];
        if (v[k] > s.max) s.max = v[k];
        sum = sum + v[k];
    }
    s.mean = sum / n;
    return s;                       // return by value
}

Stats scale(Stats s, int k) {       // pass by value: mutates only its copy
    s.min  = s.min  * k;
    s.max  = s.max  * k;
    s.mean = s.mean * k;
    return s;
}

void main(void) {
    int v[8] = { 12, 7, 25, 3, 18, 22, 9, 16 };

    Stats st = compute_stats(v, 8);
    out(0, st.min);                 // expect 3
    out(0, st.max);                 // expect 25
    out(0, st.mean);                // expect 14  (112/8)

    Stats sc = scale(st, 10);
    out(0, sc.min);                 // expect 30
    out(0, sc.max);                 // expect 250
    out(0, sc.mean);                // expect 140

    out(0, st.min);                 // expect 3   (st untouched by scale)
    out(0, st.max);                 // expect 25

    Stats cp = st;                  // copy by assignment
    cp.mean = 99;
    out(0, cp.mean);                // expect 99
    out(0, st.mean);                // expect 14  (copies independent)
}
