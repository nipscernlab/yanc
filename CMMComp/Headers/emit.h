// ----------------------------------------------------------------------------
// emit sink stack ------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// add_instr / add_sinst write to f_asm by default. A stack of capture buffers
// can be pushed on top to redirect output: while a buffer is active, the
// emitting helpers append assembly text to it instead of f_asm. The captured
// text is heap-owned and the caller picks it up via emit_pop_capture().
//
// The only remaining client today is data_declar.c: declar_arr_1d /
// declar_arr_2d / declar_Mv / declar_cv all emit `#array ...` directives (and
// in the 2D case, real LOD/SET instructions for the `_arr_size` helper) at
// parse time. They wrap their emit in push/pop and route the captured text
// through STMT_RAW so it lands in source order with the surrounding deferred
// statements when the function body walker emits.
//
// emit_raw replays a previously captured chunk through the current sink
// without re-counting f_lin / num_ins (the original add_instr calls already
// did the bookkeeping when the text was first captured).

void  emit_push_capture(void);
char *emit_pop_capture (void);

// queried by add_instr / add_sinst to pick between top capture and f_asm
int   emit_is_capturing(void);
void  emit_append      (const char *s);

// replays a pre-captured chunk without re-counting num_ins / f_lin
void  emit_raw         (const char *s);
