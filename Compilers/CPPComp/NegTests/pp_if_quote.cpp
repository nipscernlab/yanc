// #129: #if with a lone apostrophe could read past the end of the line
#if 'a
#endif
void main(void) { out(0, 1); }
