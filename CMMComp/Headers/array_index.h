// ----------------------------------------------------------------------------
// array index handling -------------------------------------------------------
// ----------------------------------------------------------------------------

// array used in assignments (left-hand side) ---------------------------------

void arr_1d_index(int id, int et);              // gets the index of a 1D array
void arr_2d_index(int id, int et1, int et2);    // gets the index of a 2D array

// array used in expressions (right-hand side) --------------------------------

int  arr_1d2exp  (int id, int et , int fft);    // turns a 1D array into an exp
int  arr_2d2exp  (int id, int et1, int et2);    // turns a 2D array into an exp
