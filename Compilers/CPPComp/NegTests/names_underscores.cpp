// C++ reserves names with __ in them; YANC uses them for its own words (__fp is the frame pointer).
#pragma yanc prname p
int __fp;
int main() { __fp = 1; out(0, __fp); return 0; }
