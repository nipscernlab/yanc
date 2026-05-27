// ----------------------------------------------------------------------------
// global routines and variables used to generate the .mif memory files -------
// as the lexer scans the .asm ------------------------------------------------
// ----------------------------------------------------------------------------

// global variables -----------------------------------------------------------

// directories used to access the .mif and .v files
extern char proc_dir[1024];    // processor directory
extern char temp_dir[1024];    // Tmp folder directory
extern char  hdl_dir[1024];    // HDL folder directory
extern char  mac_dir[1024];    // Macros folder directory

// stores the directive values
extern char prname  [128 ];    // processor name
extern int  nubits;            // ALU word width (bits)
extern int  nbmant;            // mantissa width (bits)
extern int  nbexpo;            // exponent width (bits)
extern int  ddepth;            // data stack depth
extern int  sdepth;            // subroutine stack depth
extern int  nuioin;            // number of input ports
extern int  nuioou;            // number of output ports
extern int  nugain;            // division constant
extern int  fftsiz;            // FFT size (bits)

// global functions -----------------------------------------------------------

int get_n_ins();               // number of instructions (only use after eval_init())
int inn_used(int i);           // tells whether input port i was used
int out_used(int i);           // tells whether output port i was used

// functions used in the lexer (.l) -------------------------------------------

void eval_init  (int   clk  , int clk_n);
void eval_direct(int   next_state);
void eval_opcode(int   op   , int next_state, char *text, char *nome);
void eval_opernd(char *va   , int is_const);
void eval_finish();
