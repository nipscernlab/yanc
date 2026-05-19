// ----------------------------------------------------------------------------
// CNIPS — code generator -----------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CNIPS_CODEGEN_H
#define CNIPS_CODEGEN_H

#include <stdio.h>
#include "ast.h"

void codegen(FILE *out_asm, unit *u, const char *tmp_dir);

#endif
