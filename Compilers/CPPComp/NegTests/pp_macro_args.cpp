// #130: MAX(1) became ((1)>()?(1):())
#define MAX(a, b) ((a) > (b) ? (a) : (b))
void main(void)
{
    out(0, MAX(1));
}
