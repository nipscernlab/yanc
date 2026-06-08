// ----------------------------------------------------------------------------
// CPPComp — code generator -----------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_CODEGEN_H
#define CPPCOMP_CODEGEN_H

#include <stdio.h>
#include "ast.h"

void codegen(FILE *out_asm, unit *u, const char *tmp_dir, const char *src_path);

#endif
