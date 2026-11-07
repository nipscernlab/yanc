// #130: a # in a code line ate the rest of the line, and the error moved
void main(void)
{
    int x = 3 # + 4;
    out(0, x);
}
