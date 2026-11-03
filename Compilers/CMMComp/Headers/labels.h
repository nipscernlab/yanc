// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_LABELS_H
#define CMM_LABELS_H

int push_if   (void); // the number of a new if/else    (its __Lif<n> labels)
int push_while(void); // the number of a new loop/switch (its __Lwh<n> labels)

#endif // CMM_LABELS_H
