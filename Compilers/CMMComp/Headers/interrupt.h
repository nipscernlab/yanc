// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_INTERRUPT_H
#define CMM_INTERRUPT_H

extern int itr_seen;  // the program has a #PRACA (set while parsing)

void directive_inter (void);   // marks the interrupt start point         (#PRACA)
void directive_toaqui(void);   // marks the address compared by the cheguei pin (#TOAQUI)

#endif // CMM_INTERRUPT_H
