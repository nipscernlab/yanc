// ----------------------------------------------------------------------------
// The processor's name: the source file's, and a Verilog module name ----------
// ----------------------------------------------------------------------------
// The compilers name the processor after the source file (filtro.cmm makes
// the module filtro), so the file's name has to be a Verilog identifier that
// is not a keyword; otherwise the Verilog the tools write fails only later,
// in a simulator or a synthesizer.
// ----------------------------------------------------------------------------

#ifndef PROC_NAME_H
#define PROC_NAME_H

// 0 when name can name the processor; otherwise why not:
//   1  empty, or a character other than a letter, a digit or '_'
//   2  starts with a digit
//   3  starts with "__" (the compilers' own names)
//   4  a Verilog or SystemVerilog keyword
int proc_name_problem(const char *name);

#endif
