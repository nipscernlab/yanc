// ----------------------------------------------------------------------------
// routines for generating the .v files ---------------------------------------
// ----------------------------------------------------------------------------

void hdl_vv_file(int n_ins, int n_dat, int nbopr, int itr_addr, int toaqui_addr); // creates a .v file with one processor instance
void hdl_tb_file(int itr_addr, int toaqui_addr);                                  // creates the testbench file
