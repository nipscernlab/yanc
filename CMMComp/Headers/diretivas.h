// ----------------------------------------------------------------------------
// gathers functions and variables driven by directives -----------------------
// ----------------------------------------------------------------------------

// only these 3 are used by the cmm compiler
extern char prname[128]; // processor name
extern int  nbmant;      // mantissa width (bits)
extern int  nbexpo;      // exponent width (bits)
extern int  nuioin;      // number of input ports
extern int  nuioou;      // number of output ports

// directive parsing
void dire_exec (char *dir, int id, int t);
