// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_ITR_H
#define CMM_ITR_H

extern int itr_seen;  // the program has a #PRACA (set while parsing)

void dire_inter (void);   // marks the interrupt start point         (#PRACA)
void dire_toaqui(void);   // marks the address compared by the cheguei pin (#TOAQUI)

#endif // CMM_ITR_H
