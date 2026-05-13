// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// data conversion ------------------------------------------------------------

char* itob(int x, int w);                           // converts an integer to a binary string

// helper functions for accessing terminal types ------------------------------

int  get_type   (int et);                           // gets the data type
int  get_img_id (int id);                           // gets the id of the imaginary part
void get_cmp_cst(int et, int *et_r, int *et_i);     // generates ets for a complex constant
void get_cmp_ets(int et, int *et_r, int *et_i);     // gets the ets of a complex variable
