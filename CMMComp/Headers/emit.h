// ----------------------------------------------------------------------------
// emit sink stack ------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Today add_instr/add_sinst write straight to f_asm. To migrate compound
// constructs like if/while to AST, we need to capture the assembly text
// emitted during the body parse, so we can stuff it into an ast_node and
// re-emit it later (wrapped with labels/jumps).
//
// The sink stack is the redirection layer:
//
//   - When the stack is empty, add_instr keeps writing to f_asm exactly as
//     before. This is the default (non-capturing) mode.
//
//   - emit_push_capture() pushes a fresh string buffer onto the stack.
//     While that buffer is on top, add_instr appends the assembly text to
//     it instead of f_asm. (f_lin and num_ins bookkeeping stay immediate.)
//
//   - emit_pop_capture() pops the top buffer and returns its content as a
//     heap-owned NUL-terminated string. Caller is responsible for free()
//     (or for handing it off to an ast_node via ast_raw, which copies it).
//
// This module is plumbing only - in this step it is not wired into
// add_instr yet. The next step will route add_instr/add_sinst through
// emit_is_capturing/emit_append.

void  emit_push_capture(void);
char *emit_pop_capture (void);

// used internally by add_instr/add_sinst once they are routed through here
int   emit_is_capturing(void);
void  emit_append      (const char *s);
