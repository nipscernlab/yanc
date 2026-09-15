// ----------------------------------------------------------------------------
// gathers functions and variables driven by directives -----------------------
// ----------------------------------------------------------------------------

// state the cmm compiler keeps from the directives
extern char prname[128]; // processor name
extern int  nbmant;      // mantissa width (bits)
extern int  nbexpo;      // exponent width (bits)
extern int  nuioin;      // number of input ports
extern int  nuioou;      // number of output ports
extern int  fround;      // float rounding level (#FROUND)

// directive parsing
void dire_exec (char *dir, int id, int t);
