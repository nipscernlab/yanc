// #130: a call with no ')' to the end of the file
#define ADD(a, b) ((a) + (b))
void main(void)
{
    out(0, ADD(1,
