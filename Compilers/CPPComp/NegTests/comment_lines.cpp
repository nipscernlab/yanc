// An error after a multi-line comment names its own line (#132: the
// comment's newlines counted twice, and this said line 8).
/* one
   two */
void main(void) {
    int x = ;
}
