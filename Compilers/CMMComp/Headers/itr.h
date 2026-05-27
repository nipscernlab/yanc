// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

extern int itr_ok;    // tells whether an interrupt has already been used
extern int toaqui_ok; // tells whether a #TOAQUI marker has already been used

void dire_inter ();   // marks the interrupt start point         (#PRACA)
void dire_toaqui();   // marks the address compared by the cheguei pin (#TOAQUI)
