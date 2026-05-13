// ----------------------------------------------------------------------------
// SAPHO standard library -----------------------------------------------------
// ----------------------------------------------------------------------------

// these routines should be called from the .y (parser)

// ----------------------------------------------------------------------------
// input and output -----------------------------------------------------------
// ----------------------------------------------------------------------------

int  exec_in  (int et);           //  data input
int  exec_fin (int et);           //  data input (converting to float)
void exec_out (int et1, int et2); // data output
void exec_fout(int et1, int et2); // data output (converting to float)

// ----------------------------------------------------------------------------
// special functions that save code -------------------------------------------
// ----------------------------------------------------------------------------

int  exec_sign(int et1, int et2); // takes the sign of the first argument and applies it to the second
int  exec_abs (int et);           // absolute value
int  exec_pst (int et);           // clears if negative
int  exec_norm(int et);           // division by constant
void exec_copy(int et1, int id2); // copies the value of the first argument into the second (no type checking)

// ----------------------------------------------------------------------------
// arithmetic functions -------------------------------------------------------
// ----------------------------------------------------------------------------

int  exec_sqrt(int et);           // square root
int  exec_atan(int et);           // arctangent
int  exec_sin (int et);           // sine
int  exec_cos (int et);           // cosine

// ----------------------------------------------------------------------------
// special functions for complex numbers --------------------------------------
// ----------------------------------------------------------------------------

int  exec_real(int et);           // returns the real part
int  exec_imag(int et);           // returns the imaginary part
int  exec_mod2(int et);           // squared magnitude (move to the parser)
int  exec_fase(int et);           // phase in radians
int  exec_comp(int etr, int eti); // combines two real numbers into a complex

// ----------------------------------------------------------------------------
// special functions for vector work ------------------------------------------
// ----------------------------------------------------------------------------

int  exec_vtv  (int id1, int id2);                   // multiplication between two vectors
void exec_Mv   (int idy, int idM, int idv);          // matrix-vector multiplication
void exec_cv   (int idy, int et , int idv);          // multiplication of a vector by a constant
void exec_apcb (int idy, int ida, int etc, int idb); // weighted sum into the second vector
void exec_vvt  (int idM, int ida, int idb);          // outer product between two vectors
void exec_Mmvvt(int idA, int idB, int ida, int idb); // matrix subtraction with outer product
void exec_cM   (int idA, int etc, int idM);          // product of constant and matrix
void exec_cI   (int idM, int etc);                   // generates the identity matrix
void exec_v0   (int idv);                            // generates a zero vector
void exec_cvin (int idv, int etc, int idp);          // reads an input vector
void exec_vout (int idp, int etc, int idv);          // writes a vector to the output
void exec_shift(int ida, int etb, int idc);          // shift register in vector a (a must equal c)
