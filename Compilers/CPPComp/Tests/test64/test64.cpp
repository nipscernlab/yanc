#pragma yanc prname test64
// Regression for the cur_base clobbering bug in array dimensions: a sizeof(T)
// (or cast / new) in an array's size expression reduces its own base_type and
// overwrote the global cur_base that the enclosing declarator reads afterwards,
// so `float a[sizeof(int)]` registered `a` as int[] and emitted int load/store
// semantics (assigning 1.5f stored 1). array_suffix now restores cur_base across
// the size expression. Covers the no-initializer declarator and the brace-init
// declarator (both previously read the clobbered cur_base).
void main(void) {
    // (1) no initializer -- the declarator reads cur_base directly.
    float a[ sizeof(int) + 1 ];          // size 2; must stay float[]
    a[0] = 1.5f; a[1] = 2.5f;
    out(0, (int)((a[0] + a[1]) * 1000.0f));   // 4000 (int[] bug -> 3000)

    // (2) brace initializer with sizeof in the dimension.
    float b[ sizeof(float) + 1 ] = { 0.25f, 0.75f };   // size 2
    out(0, (int)((b[0] + b[1]) * 1000.0f));   // 1000 (int[] bug -> 0)
}
