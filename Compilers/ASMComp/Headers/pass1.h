// ----------------------------------------------------------------------------
// asmcomp's first pass over the .asm (pass1.l, pass1.c) ------------------------
// ----------------------------------------------------------------------------

#ifndef PASS1_H
#define PASS1_H

// reads asm_path once and writes <dir>/app_log.txt: the directives, the
// address of every label, n_ins and n_dat (exits on an error)
void pass1_run(const char *asm_path, const char *dir);

// the scanner's actions (pass1.c)
void p1_init  (const char *dir);
void p1_direct(int next_state);
void p1_itrad (void);
void p1_toaqui(void);
void p1_opcode(int next_state);
void p1_opernd(char *va);
void p1_label (char *va);
void p1_finish(void);

#endif
