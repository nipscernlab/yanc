// #130: an #if with no #endif gave only "syntax error" on line 1
void main(void) { out(0, 1); }
#if 0
int x;
