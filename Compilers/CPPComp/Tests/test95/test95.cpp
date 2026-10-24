// Float literal suffixes (2026-10-21): C++ writes f, F, l and L; cppcomp read
// only the lowercase f, and 1.5F or 2.5L was a syntax error. There is one
// float on the SAPHO, so the suffix changes nothing.
// Expected outputs: 3 5 7 9 1
void main(void)
{
    int k = in(0);
    out(0, (int)(1.5f * 2.0f));                   // 3
    out(0, (int)(2.5F * 2.0F));                   // 5
    out(0, (int)(3.5l * 2.0l));                   // 7
    out(0, (int)(4.5L * 2.0L));                   // 9
    out(0, k + 1);                                // 1
}
