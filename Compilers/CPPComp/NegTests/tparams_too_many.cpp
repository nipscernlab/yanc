// A template of 9 parameters, one more than CPP_TPARAM_MAX (#129): cppcomp
// stops with a message that says the limit.

template <typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I>
int f(A a) { return 1; }

void main(void) { out(0, 1); }
