// relu() walks an array whose size the compiler knows: a pointer has none
float h[4];
int main() { float *p = h; relu(p); out(0, (int)h[0]); return 0; }
