// ----------------------------------------------------------------------------
// variable declaration -------------------------------------------------------
// ----------------------------------------------------------------------------

extern int type_tmp; // captures the type when a variable is declared (see c2asm.l)

// ----------------------------------------------------------------------------
// standard declarations ------------------------------------------------------
// ----------------------------------------------------------------------------

void declar_var   (int id);                                          // variable declaration
void declar_arr_1d(int id_var , int id_arg,           int id_fname); // 1D array declaration
void declar_arr_2d(int id_var , int id_x  , int id_y, int id_fname); // 2D array declaration

// ----------------------------------------------------------------------------
// array declarations initialized with Dirac notation -------------------------
// ----------------------------------------------------------------------------

void declar_Mv    (int id_name, int id_N  , int id_M, int id_v    ); // e.g. float a[4] # |B|a>;
void declar_cv    (int id_name, int id_N  , int id_c, int id_v    ); // e.g. float a[4] # c|a>;
