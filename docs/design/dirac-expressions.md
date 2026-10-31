# Dirac notation as expressions, computed in loops

Status: parked on 2026-10-26, **resumed on 2026-10-29** (Luciano, issue
#18) as a compiler change only: the loops are generated in software with
today's instruction set, and the pointers and `RPT` of section 6 are not
adopted for now (section 8). Step (a) of section 5, the grammar, the tree
and the shapes with the code still unrolled and every golden identical,
is in `main` (from commit 20112ab, tag `archive/dirac-parser`); step (b),
the loops in software, was done on 2026-10-30 (section 9); step (c), the
unrolled generators removed, on 2026-10-30 too (section 1.2 describes them
as they were; the shift register `exec_shift` stays). Decision (Luciano, 2026-10-25): the
Dirac notation of C± stops being a closed set of grammar rules, one per
form, each with a generator that unrolls the computation, and becomes
expressions in the parser: every operator a node of the tree, with shape
rules (a vector of N, an N×M matrix, a bra or a ket) and a compile error
when the shapes do not match. The code is generated as loops in software.
Choosing, later, between the software loop and the array unit
([array-unit.md](array-unit.md), parked as well) is separate work; nothing
here depends on that unit. Section 6.8 compares the two.

Marks: **fact** (read in the code, with file and line), **measured** (run on
the study's branch, method given), **deduced** (reasoned, not run).

## 1. Why the unrolled form cannot stay

### 1.1 Program size and time grow with N² (measured)

Every Dirac form is unrolled at compile time: one group of instructions per
element, no loop. To see what that costs, `proc_rls` (the RLS fixture,
`Compilers/CMMComp/Tests/proc_rls`) was compiled with every `[4]` replaced by
`[N]` and the read of `x` (`x # fin(0)*0.001 -> |x⟩;`) repeated N times, as
the fixture repeats it 4 times. Words: `n_ins` in `app_log.txt`. Clocks: a
probe on the core counts the clocks between the first `OUT` of two loop turns
(an `OUT` that follows at least 8 clocks without one). Commit 46f4067
(the `main` of 2026-10-25), built with
`make CFLAGS="-O2 -Wall -Werror"`, Icarus Verilog.

| N | words of program | bits per word | clocks per turn |
|---|---|---|---|
| 4 | 335 | 16 | 307 |
| 8 | 1091 | 18 | 1011 |
| 16 | 3947 | 19 | 3667 |
| 32 | 15035 | 21 | 13971 |
| 128 | 232091 | 25 | not simulated |

At N = 4 the output is the fixture's golden, bit for bit.

The two series follow

- words = 14N² + 21N + 27,
- clocks per turn = 13N² + 20N + 19,

exactly at every point measured (words at all five, clocks at N = 4, 8, 16
and 32; the clock formula, fitted on 4, 8 and 16, predicted the 13 971 of
N = 32 before it was simulated). At N = 32 the 400 values of the input file
run out after the first turn; the clocks do not depend on the data, since
no branch of the program does. At N = 128 the clock formula gives 215 571 clocks per turn
(deduced). The N² terms are the unrolled forms: per turn, `|P|x⟩` costs 3
instructions per element of P, `|P| - |K⟩⟨Px|` 5, `1.010101|P|` 3, and the N
shifts of `x` 2 per element each, 13 in all; the words add the one-time
`1000.0|I|`.

At N = 128 the program is 232 091 words of 25 bits, 5 802 275 bits. The
DE10-Nano's Cyclone V has 553 M10K blocks (the count Quartus reports for the
device in the fits of the array-unit work), 553 × 10 240 = 5 662 720 bits:
the program memory alone would need more block memory than the whole chip
has, before the data memory (16 908 words at N = 128). Compiling it took
43 s.

### 1.2 Every combination is a rule and a generator (fact)

(Before step b; the generators below were removed in step c, 2026-10-30,
and the line numbers are those of that time.) There are no Dirac
expressions in the parser. Each form is a statement rule
of its own in `Compilers/CMMComp/Sources/CMMComp.y`, built into a
`STMT_DIRAC` node tagged with one `dirac_op` (`Compilers/CMMComp/Headers/ast.h`
244-255), dispatched in `stmt_emit` (`Compilers/CMMComp/Sources/ast.c`
1289-1300) to one generator in `Compilers/CMMComp/Sources/stdlib.c`:

| form | rule (`CMMComp.y`) | generator (`stdlib.c`) |
|---|---|---|
| `⟨a\|b⟩` (an `exp`) | 509 | `exec_vtv`, 2625 |
| `v # \|M\|a⟩` | 426 | `exec_Mv`, 2705 |
| `float v[N] # \|M\|a⟩` (declaration) | 207 | `exec_Mv` |
| `v # c\|a⟩` | 427 | `exec_cv`, 2803 |
| `float v[N] # c\|a⟩` (declaration) | 208 | `exec_cv` |
| `v # \|a⟩ + c\|b⟩` | 428 | `exec_apcb`, 2885 |
| `M # \|a⟩⟨b\|` | 429 | `exec_vvt`, 2976 |
| `M # \|A\| - \|a⟩⟨b\|` | 430 | `exec_Mmvvt`, 3053 |
| `M # c\|A\|` | 431 | `exec_cM`, 3141 |
| `M # c\|I\|` | 432 | `exec_cI`, 3218 |
| `v # \|0⟩` | 433 | `exec_v0`, 3287 |
| `v # c\|in(p)⟩` | 434 | `exec_cvin`, 3324 |
| `out(p, c\|a⟩)` | 340 | `exec_vout`, 3390 |
| `a # c -> \|a⟩` | 435 | `exec_shift`, 3460 |

The generators take about 940 lines of `stdlib.c` (2617 to 3562), most of it
the same checks repeated in each. A new combination, `|a⟩ - |b⟩` or
`|A| + c|a⟩⟨b|` or `⟨a|M|b⟩`, needs a rule, a `dirac_op`, a constructor, a
case in `stmt_emit`, a generator with its checks and its type cases, a
prototype in `stdlib.h` and an `MSG_INFO_DIRAC_*` message: about eight places
for each form, and the forms multiply with every operator added. The
issue #18 lists seven families still missing.

## 2. Defects and limits known today

The full survey is in issue #18. Measured on the study's branch with small
programs compiled and simulated (2026-10-25):

| what | expected | today | where |
|---|---|---|---|
| `A # \|a⟩⟨b\|`, a = (1, 2), b = (10, 30) | 10 30 20 60 | 10 20 30 60 | `exec_vvt` writes `N*j+i` (`stdlib.c` 3046) |
| `A # \|B\| - \|a⟩⟨b\|`, B = (100 200; 300 400) | 90 170 280 340 | 90 180 270 340 | `exec_Mmvvt` reads and writes `N*j+i` (3124, 3135): it computes \|B\| − \|b⟩⟨a\| |
| `a # \|M\|a⟩`, M = (0 1; 1 0), a = (1, 2) | 2 1 | 2 2 | `exec_Mv` writes `a[0]` before the second row reads it |
| `int a[3]; a # k+1 -> \|a⟩;`, k = 5 | 6 0 0 | 0 0 0 | `exec_shift` has no case for an int vector and an int in the accumulator (3505-3559): it emits `LOD k; ADD 1` and nothing else, without an error |

All four are fixed by step (b) (section 9), and the fixture `dirac_shapes`
checks each against values computed apart from YANC.

The fixtures missed the first three: in `proc_rls` the outer product is
K Pxᵀ with K = g Px, symmetric up to rounding, and `|P| - |K⟩⟨Px|` writes P
in place at the same element; `dirac_assign` never outputs a matrix and its
`a # |B|a⟩` has B = I.

Limits, from the code (fact) unless marked:

- no `comp` in any form (`MSG_ERR_NOT_IMPL_COMPLEX` in every generator);
- the scalar must have the vector's type: no int/float mix
  (`MSG_ERR_VARS_SAME_TYPE`);
- with the scalar 1, every form still multiplies: `out(0, 1|a⟩)` is
  `LOD_V`, `MLT 1`, `OUT` per word, and the multiplier is synthesized for it;
  `out(p, |a⟩)` without a scalar is a syntax error (issue #18);
- the outer product only square, a, b and A of one size, with no weight;
- `exec_cI` indexes `N*i+j` with N the number of rows and never checks that
  the matrix is square (deduced: a non-square matrix gets the wrong cells);
- the declaration forms of 1.2 are not in the manual (`docs/library.md`,
  "Vectors in Dirac notation", lists only the assignment forms);
- the grammar has one shift/reduce conflict today, with no `%expect`:
  bison 3.8.2 reports it in the state of `A # c|B|;`, where
  `c | B | ...` could also be a bitwise or. Bison shifts, so the Dirac
  reading wins. The same ambiguity is the main problem of section 3.1.

## 3. The proposal

### 3.1 Grammar

The notation stays as it is written today. A Dirac term is a chain of names
between bars: `⟨a|`, `|M|`, `|a⟩`, `⟨a|M|b⟩`, `|A|B|x⟩`. Juxtaposition is the
product: `c|a⟩`, `|a⟩⟨b|`, `|A||B|`. Sums and differences use `+` and `-`.
In a chain, the first name is a bra (row vector) when the chain opens with
`⟨`, the last is a ket (column vector) when it closes with `⟩`, and every
other name is a matrix. `|I|`, `|0⟩` and `|in(p)⟩` stay as they are.

The difficulty is the bar. C± keeps C's bitwise or, so `a | b | c` must still
parse as `(a | b) | c`, and the parser must tell it apart from `c|M|v⟩`.
The two token strings are the same up to the last token, so no grammar with
one token of lookahead can decide where the coefficient ends. Three ways out
were compared, each as a bison prototype with the operators and precedences
of `CMMComp.y` (measured, bison 3.8.2):

| | what | conflicts | for | against |
|---|---|---|---|---|
| G1 | keep one statement rule per form, add the missing ones | 1 today, more with each form | no lexer change | the problem of section 1.2 |
| G2 | a separate nonterminal `lin` for vector and matrix values, only after `#` and in `out()`; `⟨...⟩` scalars inside `exp` | 1 (the same as today) | no lexer change | vector values only at statement level; a scalar coefficient with `\|` in it is read as a chain; `out(p, ...)` needs two rules sharing a prefix |
| G3 | Dirac terms inside `exp`; the lexer returns a distinct token for a bar followed by the name of an array | 0 | one grammar for scalars and vectors; any context; precedence of the product set apart from `\|` | the lexer looks at the symbol table |

**Recommended: G3.** The lexer already resolves every name against the
symbol table when it reads it (`exec_id`, `Compilers/CMMComp/Sources/variaveis.c`
232, with the function's scope), so the information is at hand (fact). The
rule: a `|` followed, after blanks, by the name of an array (a variable with
`isar` set) that is not followed by `[` is the token `DBAR`; any other `|`
is the bar it is today (bitwise or, or the bar that closes a matrix or a
bra). An array's name without an index is not an operand today (measured:
`int y = 3 | a;` with `int a[4]` stops with "where's the array index for
the variable 'a'?"), so no program that compiles today changes meaning.
With an index it is (`c | a[0]`): the first version of the rule missed
that, and the fixture `NegTests/dirac_or_index.cmm` keeps it. `|in(p)⟩` is lexed whole, like `|0⟩` today,
because `x | in(0)` is a valid or of an input.

The grammar G3 adds to `exp`:

```
exp   : ... | dirac | exp dirac %prec JUXT
dirac : DBAR ID tail | KET ID tail | EYE | VZERO | KETIN
tail  : BRA | '|' | DBAR ID tail
```

with the product's precedence between `+ -` and `* / %`:

```
%left '|'            ...  (as today, lower)
%left '+' '-'
%left JUXT DBAR KET EYE VZERO KETIN
%left '*' '/' '%'
```

so `2*c|a⟩` is `(2*c)|a⟩` (the coefficient computed first, as today's rule
`exp '|' ID BRA` does), and `|b⟩ + c|a⟩` is `|b⟩ + (c|a⟩)`. A prototype
parser with that lexer rule (arrays named by a fixed list) printed these
trees (measured):

| source | tree |
|---|---|
| `\|w⟩ + e\|K⟩` | `(+ [w> (. e [K>))` |
| `\|P\| - \|K⟩⟨Px\|` | `(- [P] (. [K> <Px]))` |
| `1.010101\|P\|` | `(. 1.010101 [P])` |
| `⟨x\|P\|x⟩` | `<x P x>` (a scalar) |
| `⟨x\|P\|` | `<x P]` (a bra) |
| `\|A\|\|B\|` | `(. [A] [B])` |
| `\|A\|B\|x⟩` | `[A B x>` |
| `2*c\|a⟩` | `(. (* 2 c) [a>)` |
| `-c\|a⟩` | `(. (neg c) [a>)` |
| `\|A\| + c\|a⟩⟨b\|` | `(+ [A] (. (. c [a>) <b]))` |
| `c \| d \| e` | `(or (or c d) e)` |
| `1.0/(0.99 + ⟨x\|P\|x⟩)` | `(/ 1.0 (+ 0.99 <x P x>))` |

and every form of section 1.2 with the tree its generator computes today.
The statements become two: `ID '#' exp ';'` and, for the declaration,
`TYPE ID '[' INUM ']' '#' exp ';'`; `out(p, exp)` takes a scalar or a vector
by its shape. The shift register `a # c -> |a⟩` is not linear algebra and
stays a statement of its own.

Risks of G3 (deduced): a `#define` name after a bar is expanded after the
lexer has decided; the lexer reads one token ahead of the parser, so an
array declared and used after a bar in the same statement would not yet be
an array (no such statement exists: the declaration forms use the new name
before `#`).

### 3.2 Shapes

Every node of the tree gets a shape (rows, cols); a scalar is (1, 1), a ket
of N is (N, 1), a bra of N is (1, N), an N×M matrix is (N, M). A 1D array is
a ket or a bra by its place in the chain; a name in a matrix place must be
2D, a name in a vector place 1D. The rules, checked bottom-up in a pass like
`typecheck_expr` (`ast.c` 327):

| node | shapes | result |
|---|---|---|
| `X + Y`, `X - Y` | equal | the same |
| `-X` | any | the same |
| `c X` (juxtaposition, c a scalar) | any X | X's |
| `X Y` (juxtaposition, both not scalar) | X (n, k), Y (k, m) | (n, m); k = 1 is an outer product, k > 1 a sum over k |
| `\|I\|`, `\|0⟩`, `\|in(p)⟩` | take the size the context needs | |
| `v # X` | v 1D of N: X (N, 1) or (1, N); v N×M: X (N, M) | |
| `x = X`, `if (X)`, a scalar operand | X (1, 1) | |
| `out(p, X)` | X (1, 1), (N, 1) or (1, N) | |

A mismatch is a compile error naming the line, the two operands and their
shapes. Messages, each in `messages.h` with `M(pt, en)`; the names reuse
what exists (`MSG_ERR_NOT_A_VECTOR`, `MSG_ERR_NOT_A_MATRIX`,
`MSG_ERR_DIM_MISMATCH`) and add, as a proposal:

| name | English text |
|---|---|
| `MSG_ERR_DIRAC_SUM_SHAPE` | line %d: %s (%s) and %s (%s) do not have the same shape |
| `MSG_ERR_DIRAC_PRODUCT_SHAPE` | line %d: %s (%s) times %s (%s): %d columns against %d rows |
| `MSG_ERR_DIRAC_TARGET_SHAPE` | line %d: %s is %s and the expression is %s |
| `MSG_ERR_DIRAC_NOT_SCALAR` | line %d: a %s where a number is needed |
| `MSG_ERR_DIRAC_OUT_MATRIX` | line %d: out() writes a number or a vector, and %s is %s |
| `MSG_INFO_DIRAC_TEMP` | line %d: %s is read and written with a different index: the result goes through the temporary area (%d words) |
| `MSG_INFO_DIRAC_TEMP_TOTAL` | at the end of the compilation, when the area exists: its size in words, the lines that need it with the words of each, and how to avoid it (3.4, requirement 4) |

with a shape written as "a vector of 4", "a row of 4", "a 4x3 matrix" (in
Portuguese "um vetor de 4", "uma linha de 4", "uma matriz 4x3"). The
element types follow the scalar rules of `typecheck_expr` (int, float,
comp), so the mix of int and float is allowed where `+` and `*` allow it
today. With `comp`, a bra conjugates. The texts in `messages.h` are not
these: on 2026-10-29 Luciano approved versions in the casual tone of
cmmcomp's other messages ("adding a vector of 4 and a 2x2 matrix? Those
shapes don't fit, mate!").

### 3.3 Into the expression tree (fact, then proposal)

What the tree has today:

- `expr_node` (`ast.h` 128-143) carries a kind, a type, a v_table id and two
  children; there is no shape. Codegen is deferred: `ast_emit_expr`
  (`ast.c` 517) walks the tree post-order and calls the scalar helpers.
- What the walker returns is an `expr` (`ast.h` 42-45): a type and the place
  of a single value, the accumulator (id 0) or a variable. A vector or
  matrix value has nowhere to live in that result.
- The only Dirac node is `EXPR_INNER` (`ast.h` 68), whose children must be
  plain variables (`ast.c` 742 reads `n->left->id`, `n->right->id`) and
  which calls the unrolled `exec_vtv`.
- The loops a lowering needs already exist as nodes: `STMT_WHILE` with the
  `for` desugaring and its bottom test (`for_bottom_test`, `ast.c` 269),
  `STMT_ARRAY_ASSIGN` and `EXPR_ARRAY_INDEX` with an index expression, which
  emit `LDI`/`STI` (seen in the assembly of section 3.5).
- Literal 1 already drops out of a multiplication (`ast.c` 569-571) when the
  surviving operand has the result's type.

Proposal: add `rows` and `cols` to `expr_node` and four kinds,
`EXPR_DIRAC_REF` (an array in a chain, with its role), `EXPR_DIRAC_PROD`
(juxtaposition), `EXPR_DIRAC_CONST` (`|I|`, `|0⟩`) and `EXPR_DIRAC_IN`
(`|in(p)⟩`); `+`, `-` and unary `-` stay `EXPR_BINOP`/`EXPR_UNOP`, with
`shapecheck_expr` filling the shape. `EXPR_INNER` becomes the case of
`EXPR_DIRAC_PROD` whose result is (1, 1). `STMT_DIRAC`, `dirac_op`, the ten
`stmt_dirac_*` constructors and the twelve generators of section 1.2 except
`exec_shift` go away. A vector-shaped tree is never handed to `ast_emit_expr`:
it is lowered first (3.4).

### 3.4 Code generation

**Superseded by section 6** (Luciano, 2026-10-25): the loops are to take
the fewest instructions possible, with small hardware if needed, so L1
below (the generic walker, 12 to 20 instructions an element) is replaced by
a generator of its own for the Dirac loops. The rest of this section (the
requirements on scalars, aliasing, the temporary area) still holds. That
generator, with today's instructions only, is section 9.

Three ways were weighed:

| | how | for | against |
|---|---|---|---|
| L1 | lower each Dirac statement to the loops and indexed accesses the tree already has (`for`, `x[k]`, `x[k][j]`), and let the existing walker emit them | every combination at once; the scalar walker's types, mixing and folding; one code path to test; the line tables point at the Dirac line | the code is what the general array code gives (3.5) |
| L2 | one hand-written loop emitter per operator, with `LDI`/`STI` | could shave instructions | back to one generator per operator, each with its type cases |
| L3 | keep unrolling below a size | fastest for small N | two code paths; the decision of 2026-10-25 is loops |

**Recommended: L1**, with these requirements:

1. **The value of each element is a scalar expression tree.** For a target
   element (r) or (r, c): a ket `|a⟩` gives `a[r]`, a bra `⟨a|` gives `a[c]`,
   `|M|` gives `M[r][c]`, `c X` gives `c * X(r, c)`, `X + Y` gives
   `X(r, c) + Y(r, c)`, an outer product `|a⟩⟨b|` gives `a[r] * b[c]`, and a
   product with k > 1 becomes an accumulator: `s = X(r, 0) * Y(0, c)`, then
   `s = s + X(r, j) * Y(j, c)` for j = 1 to k−1, in that order (section 4
   says why the order matters).
2. **Scalar subtrees are computed once, before the loops**, into a compiler
   temporary (as `exec_cv` does with `__aux_var`, `stdlib.c` 2857). That
   keeps today's semantics (`fin(0)` in a coefficient is read once) and
   keeps calls out of the loops. A literal stays in place, so the folding of
   `* 1` removes `1|a⟩`'s multiply (issue #18).
3. **No `MLT` for an index.** The 2D access of the general array code
   computes `r*N + c` with `MLT` (`LOD r; MLT 4; ADD c; LDI P`, measured in
   3.5), which synthesizes the integer multiplier in a program that may use
   only floats: measured, the loop version of `proc_rls` turns on `MLT` and
   `SF_MLT` in its processor, the Dirac version only `F_MLT`. The lowering
   keeps a running row offset (an `ADD` of N per row), and an element-wise
   matrix operation (`c|A|`, `|A| - |B|`, the zero) runs over one flat index
   of N·M. The area of the extra multiplier was not measured.
4. **Aliasing.** When the target is read with a different index from the
   one written (`a # |M|a⟩`, `P # |P||Q|`, a transpose), the result is
   computed into the temporary area and copied to the target at the end,
   with `MSG_INFO_DIRAC_TEMP` on that line. Read at the same index
   (`P # |P| - |K⟩⟨Px|`, the case of `proc_rls`) needs nothing: each element
   reads only itself before it is written.
5. **Chains are grouped to keep the work at N²**: `|A|B|x⟩` is `A (B x)`,
   with `B x` in the temporary area, not `(A B) x`. A chain of three
   products alternates two blocks of the area (t1 = C x, t2 = B t1,
   target = A t2).
6. **One temporary area for the whole program (Luciano, 2026-10-25).** A
   temporary lives only inside one Dirac statement, and no two statements
   can be inside each other: C± has no recursion, and the scalar subtrees,
   function calls included, are computed before the loops (requirement 2).
   The interrupt (`#PRACA`) saves no context and does not come back to the
   point it left (Luciano, 2026-10-25; fact: `itr` forces the program
   counter to `ITRADD` and no return address is kept, `SAPHO/core.v` 152).
   So the compiler declares one global array,
   `__dirac_tmp`, whose size in words is the largest, over the statements
   of the program, of the blocks one statement has live at once (a block
   of N for a vector, N·M for a matrix; with `comp` later, two words an
   element). int and float share it: a word is a word, the type is in the
   instructions. A program with no aliasing and no chain longer than one
   product has no area: no data memory is spent (pay only for what you
   use). The loop counters, the derived indexes and the parts computed
   before the loops (`__dk`, `__dd`, `__dh`) are numbered inside a
   statement and carry the name of the function (`__dk3_main`): a function
   called from a coefficient has its own and leaves the caller's alone
   (corrected 2026-10-30, section 9). All these names start with `__`
   (asm-names.md, rule 2).

   At the end of the compilation, when the area exists, cmmcomp prints
   `MSG_INFO_DIRAC_TEMP_TOTAL`: the size of the area, each line that needs
   it with its words, and how to avoid it, by cause:

   - target read at another index (`a # |M|a⟩`): write the result to an
     array that is not on the right-hand side, if the program has one free
     at that point (`t # |M|a⟩;`, then use `t`);
   - a chain of products (`|A|B|x⟩`): if the program keeps `B x` anyway,
     compute it into an array of its own in a statement before
     (`y # |B|x⟩; v # |A|y⟩;`).

   The hint saves memory only when the user has an array that is free at
   that point; otherwise the area is the cheapest place, and the message
   says so.

### 3.5 Cost against the unrolled form

To have a measured stand-in for L1, `proc_rls` was written by hand with
`for` loops and indexed accesses in C±, element by element in the order of
today's generators (the transposed index of `exec_Mmvvt` kept), and
compiled by today's cmmcomp. This is the code the general array path gives,
the code L1 would hand to the walker before requirement 3.

| N | Dirac today: words | loops: words | Dirac today: clocks per turn | loops: clocks per turn |
|---|---|---|---|---|
| 4 | 335 | 289 | 307 | 1227 |
| 16 | 3947 | 469 | 3667 | 15 975 |
| 32 | 15 035 | 709 | 13 971 | 61 623 |
| 128 | 232 091 | 2149 | 215 571 (formula) | 959 127 (formula) |

All measured except the two marked; the loop version at N = 8 gave 349 words
and 4287 clocks. The loop series follow words = 15N + 229 (the 15N is the
test's own N copies of the read of `x`, each a loop of fixed size) and
clocks = 58N² + 69N + 23, fitted on N = 4, 8 and 16 and exact at N = 32
(61 623 predicted and simulated). At N = 4 the loop version's output is the
golden, and the bits of P and w after the 20 turns are the Dirac version's
(section 4).

Per element, read in the loop version's assembly (fact) against the
unrolled code: inner product 12 instructions against 3, a row of `|P|x⟩` 14
against 3, the outer-product update of P 20 against 5, `1.010101|P|` 14
against 3, one step of the shift 10 against 2, one word of `out` 10 against
4. Both versions run close to one instruction per clock: the N² terms of the
clock formulas (13 and 58) are the sums of these counts (deduced from the
match).

What this says:

- Program size stops depending on N: at N = 128, 2149 words of 22 bits
  (47 278 bits) against 5.8 Mbit. Already at N = 4 the loops are smaller.
- Time per turn is about 4 to 4.5 times the unrolled code's, for every N.
  That is the price of loops on this instruction set: a load through `LDI`
  needs the index in the accumulator, and the counter costs five
  instructions a turn (`LOD k; ADD 1; SET k; LES; JIZ`). The array unit of
  array-unit.md is one way to win that back, section 6 another; this work does not try to.
- With requirement 3, the matrix loops lose the `MLT` and the index
  arithmetic: deduced, the element-wise ones from 14 to about 10
  instructions per element. Not measured.
- A hand-written emitter (L2) for the inner product could keep the sum on
  the stack and save one instruction in twelve (deduced), not enough to
  justify a second code generator.

## 4. Tests

**Same output, bit for bit.** `dirac_assign` and `proc_rls` must give the
same `golden_sim` after the change; their `golden.asm` change, on purpose
(loops instead of unrolled code), and the pull request says so. That holds
if every element is computed with the same operations on the same operands
in the same order. Where the order can change:

1. **The sum over k** of an inner product or of a matrix-vector product:
   today from k = 0 upwards, `((p0 + p1) + p2) + ...`. The lowering keeps
   that order. A loop counting down, or a sum split in two halves, would
   change the bits.
2. **Operand order inside an element.** Today's generators write the
   element first and the scalar second (`LOD_V v k; F_MLT c`, that is
   `v[k]*c`), `K[k]*e + w[k]` for `|w⟩ + e|K⟩`, and `-(a[i]*b[j]) + B` for
   `|B| - |a⟩⟨b|`. A lowering that follows the source writes `c*v[k]`,
   `w[k] + e*K[k]` and `B - a[i]*b[j]`. Measured, with no `#FROUND` (level
   0, as `proc_rls`): over 500 pairs of random floats, `x*y` and `y*x`,
   `x+y` and `y+x`, `x-y` and `-y+x` gave the same bits every time; and the
   loop version of `proc_rls` with `P - K*Px` in place of `-(K*Px) + P`
   left P and w bit for bit as the Dirac version does (and saved 16 clocks
   a turn: one `SF_SU2` per element instead of `F_NEG` and `SF_ADD`). So the lowering may
   write the source's order. Not checked at `#FROUND 1` and 2, whose
   subtraction is not the same circuit (fround1-subtraction.md): the
   fixture runs at level 0, so its golden is not at risk, but the claim is
   only made for level 0.
3. **The outer product's index.** Fixing the transposition of
   `exec_Mmvvt` changes which product is subtracted where: with K = g Px,
   the element (r, c) becomes `(Px[r]*g)*Px[c]` instead of
   `(Px[c]*g)*Px[r]`. Measured at N = 4, by writing the bits of P and w
   (through `copy` to an int) after the 20 turns: with the index fixed, 19
   of the 20 words differ from today's, in the last bits. The fixture's
   output, `w*1000` converted to int, is still the golden at N = 4 (and
   with the source's order as well). So `proc_rls/golden_sim` should not
   change, but that is a property of this output, not of the arithmetic;
   the pull request says so, and if a later N or input shows a difference,
   it is the fix and not a regression.
4. **`dirac_assign`** outputs only the constant 1, so its `golden_sim` holds
   whatever the Dirac code does. Its `a # |B|a⟩` with B = I goes through the
   temporary of requirement 4.

**A new fixture, `dirac_shapes`**, with small integers and floats that are
exact in the float format (so the expected values are written by hand, not
taken from YANC), outputting every element:

- `A # |a⟩⟨b|` with a = (1, 2, 3) and b = (10, 30): 10 30 20 60 30 90, the
  outer product in the right order and not square;
- `A # |B| - |a⟩⟨b|` and `A # |B| + 2|a⟩⟨b|` with B ≠ Bᵀ;
- `a # |M|a⟩` with M a permutation (the aliasing);
- `v # |a⟩ + |b⟩`, `v # |a⟩ - |b⟩`, `y = ⟨a|M|b⟩`, `v # ⟨a|M|` with M not
  square;
- the int shift `a # k+1 -> |a⟩`;
- `out(0, 1|a⟩)` and `out(0, |a⟩)`, the `golden.asm` showing no `MLT` and
  the processor no multiplier (issue #18);
- int with float: `float v[3] # k|a⟩` with `int k`.

And a fixture of compile errors, one per message of 3.2, each checked for
its line and its text in both languages.

## 5. Decisions (Luciano, 2026-10-25)

1. Grammar G3: Dirac terms inside `exp`, the lexer's `DBAR` for a bar
   followed by the name of an array.
2. Code generation L1: lowering to the loop nodes of the tree; the
   `exec_*` generators go, except `exec_shift`. **Reopened the same day:
   section 6.**
3. Aliasing: through the temporary area, with an info line on the
   statement and the total at the end of the compilation, with hints
   (3.4, requirements 4 and 6).
4. `out(p, X)` refuses a matrix for now (`MSG_ERR_DIRAC_OUT_MATRIX`).
5. `comp` in a later step of its own (the conjugation of a bra and a
   fixture).
6. No unrolling for small N (L3): one code path. At N = 2, `out(0, |a⟩)`
   unrolled would be 4 words and 4 clocks, the loop about 10 words and 20
   clocks (deduced); to be measured later if it matters.
7. The shape names of 3.2, in English and Portuguese.
8. Three steps, each diff shown before its commit: (a) the tree, the shapes
   and the grammar, the present fixtures giving the same output; (b) the
   lowering to loops, the `golden.asm` of `dirac_assign` and `proc_rls`
   changing on purpose; (c) the generators removed, the fixture
   `dirac_shapes` and the fixture of errors.
9. This note reaches `main` through its own pull request before the code.
10. The measured defects of section 2 are fixed by the redesign, not by a
    patch to the unrolled generators; the CHANGELOG warns about them.

## 6. Loops with small hardware (prototype, 2026-10-25 and 26)

Luciano asked for the fewest instructions in the loops, adding
instructions if needed, within these limits (2026-10-25): **no memory of
its own** (no second port, not the array unit of array-unit.md: that is the
coprocessor mode, which a parameter will choose instead of this), only
**small hardware** (registers, counters, multiplexers), every block in a
`generate` and built only when the program uses it, the **logic added
measured**, and **no path longer than today's path through the float
adder**: the clock must not drop.

Everything here is a prototype outside `main`, kept whole in the tag
`archive/dirac-loops-proto` under `.smoke/dirac-proto/`: a modified copy of
`SAPHO/` (`proto/SAPHO`, made by `proto/patch_core.py`), a bench
(`proto/bench.py`) and the fit scripts (`proto/map.sh`, `proto/fit.sh`,
`proto/sta.sh`). No compiler emits the new instructions yet: the bench
writes them by hand.

### 6.1 What the pipeline allows (fact)

Read in `SAPHO/core.v`, `SAPHO/instr_dec.v` and `SAPHO/processor.v`:

- every instruction takes one clock, a jump or `JIZ` included: the jump is
  decided in the decode and the fetch takes the target in the same cycle
  (`prefetch`, `core.v` 95-157);
- the data memory has one read port and one write port, with their own
  addresses (`mem_addr_rd`, `mem_addr_wr`);
- an instruction is decoded in cycle t (its read address leaves then), the
  word read arrives in t+1, the ALU works in t+1 (the ALU operation is
  registered in `instr_dec`) and the acc is written at the end of t+1;
- a write happens in the decode cycle and stores `ula_out`, which in that
  cycle is the result of the **previous** instruction: that is how `SET`
  works;
- `LDI base` reads `mem[base + ula_fast]`; `STI base` writes the acc to
  `mem[base + stack top]` and pops (`mem_ctrl`, `core.v` 448-481).

One ALU means one operation a clock. With one read port, an element of an
inner product costs at least three instructions (load, multiply, add),
which is what today's unrolled code takes. The write port is idle in most
instructions: that is the room left.

### 6.2 Software only, measured first

Before the hardware, the best loops with today's instructions, written by
hand, simulated, the same bits as the compiler's (inner product of 16, 6
turns; `y # |M|v⟩` of 16x16, 3 turns):

| code | instructions an element | words | clocks a turn |
|---|---|---|---|
| inner product, the compiler's `for` loop | 12 | 57 | 199 |
| sum on the stack, negative index, `JIZ`+`JMP` | 10 | 55 | 168 |
| the same, unrolled by 5 | 6.8 | 79 | 120 |
| `\|M\|v⟩`, unrolled today | 3 | 826 | 941 |
| `\|M\|v⟩`, rows in a loop, columns unrolled (`LDI M+j`) | 4.6 | 137 | 1358 |

The negative index (from -N to -1, the base at the end of the array) lets
`JIZ` close the loop; the column unrolling makes the code grow with N, not
N². Both need an assembler alias `LDI_V`/`STI_V` (base plus a constant), no
hardware.

### 6.3 The design

Three blocks, each in a `generate` on a parameter asmcomp would set from
the program (`NPTR`, `RPT`):

1. **Four pointers P0 to P3**, each as wide as a data address. With
   `NPTR > 0`, the top bit of a memory instruction's operand selects the
   pointer mode: `[1:0]` k, the pointer of the instruction's own access,
   `[3:2]` its change after the access (01 +1, 10 -1). asmcomp widens the
   operand by one bit, so no other operand reaches that bit (an invariant
   of the assembler, not checked by the hardware). `SETP k a` (opcode 110)
   loads pointer k with the address a, an immediate (every Dirac pointer
   starts at a constant address).
2. **A parallel store of the previous result**: in pointer mode, `[4]` also
   stores `ula_out` (the result of the previous instruction, what `SET`
   stores) through pointer `[6:5]` j, changed by `[8:7]`. It uses the idle
   write port. The compiler pipelines the loop: the load of element i
   stores the result of element i-1, with one instruction before and one
   after the loop.
3. **A zero-overhead loop**: `RPT L n` (opcode 111) runs the L instructions
   after it n times with no jump instruction. Three registers (the first
   address of the body, the address after it, the turns left) and a flag;
   when the PC is about to fetch the address after the body with turns
   left, the fetch takes the first address. One level; the outer loop of
   a matrix stays in software. No jump inside the body (a contract of the
   compiler). The operand holds L in 3 bits and n below it.

Opcodes 108 and 109 stay for `DOT`/`DOB` of array-unit.md. Every select reads
registers and the instruction word, never the ALU: the pointer feeds the
**base** input of the `LDI`/`STI` adder, whose other input (`ula_fast`)
comes much later, and the `RPT` choice is ready before the decode decides
a jump (`pc_load` stays the last select of the fetch address).

### 6.4 Measured in simulation

`proto/bench.py`: each form compiled by cmmcomp as today (the reference),
the unrolled stretch replaced by the hand-written loop, both simulated on
the same inputs (2 turns, N = 16), the output compared bit for bit, the
clocks of the stretch counted by a probe on the fetch address. **All 11
give the same bits.** Today's element order is kept, the transposed outer
product included, so that the bits can be compared.

| form | words today -> prototype | clocks today -> prototype | the loop body |
|---|---|---|---|
| `⟨a\|b⟩` | 48 -> 9 | 48 -> 51 | `P_LOD @0+; F_MLT @1+; SF_ADD` |
| `\|M\|v⟩` | 768 -> 18 | 768 -> 884 | the same, a row a turn of a software loop |
| `c\|v⟩` | 48 -> 8 | 48 -> 36 | `LOD @0+ >1+; F_MLT c` |
| `\|w⟩ + e\|K⟩` (in place) | 64 -> 11 | 64 -> 53 | `LOD @0+ >2+; F_MLT e; F_ADD @1+` |
| `\|P\| - \|K⟩⟨Px\|`, `F_NEG; F_ADD` | 1280 -> 24 | 1280 -> 1189 | 4 a element |
| `\|P\| - \|K⟩⟨Px\|`, `F_SU2` | 1280 -> 22 | 1280 -> 933 | `LOD @0+ >2+; F_MLT t; F_SU2 @1+` |
| `c\|P\|` | 768 -> 8 | 768 -> 516 | `LOD @0+ >1+; F_MLT c` |
| `\|a⟩⟨b\|` | 768 -> 19 | 768 -> 676 | `LOD @0+ >2+; F_MLT t` |
| `\|0⟩` | 17 -> 4 | 17 -> 19 | `SET @0+` |
| `c -> \|a⟩` | 32 -> 8 | 32 -> 21 | `LOD @0- >1-` |
| `out(0, c\|a⟩)` | 64 -> 6 | 64 -> 66 | `LOD @0+; F_MLT c; F2I; OUT 0` |

`F_SU2` is memory minus acc (`ula.v` 1626-1627: `F_SU1` turns the sign of
the memory input, `F_SU2` that of the acc); `F_SU1` gave other bits. With
`F_SU2` the bits are those of today's `F_NEG; F_ADD`, at `#FROUND 0`; not
checked at levels 1 and 2. `|M|v⟩` is 15 % slower than unrolled: 7
instructions of software loop a row; a second level of `RPT` would remove
them (not designed). The prototype core with `NPTR = 0` and `RPT = 0`
gives the output and the clocks of the original (checked on `|M|v⟩`), and
`sapho_all` with the blocks on and unused gives its golden output.

### 6.5 Measured in Quartus

`sapho_all` (every block in, `#FROUND 2`) against `sapho_ptr`, the same
program with the operand widened from 11 to 12 bits, `NPTR = 4`, `RPT = 1`,
on the DE10-Nano (5CSEBA6U23I7), `fmax.sh`, seed 1, 2026-10-25:

| | `sapho_all` | `sapho_ptr` | difference |
|---|---|---|---|
| Fmax | 32.11 MHz | 31.82 MHz | -0.9 % |
| logic (fit) | 3545 ALMs | 3643 ALMs | +98 (+2.8 %) |
| logic (synthesis estimate) | 3378 ALMs | 3467 ALMs | +89 (+2.6 %) |
| registers | 1193 | 1253 | +60 |
| program memory bits | 21,054 | 22,109 | +1055 (one bit a word) |

The worst path is the same chain in both: the acc or the ALU input, the
exponent alignment, the float adder, the normaliser, the ALU output mux,
the stack; 30.88 ns of data path in `sapho_all`, 30.76 ns in `sapho_ptr`.
None of the 3000 worst paths of `sapho_ptr` (down to -20.6 ns of slack at
the 10 ns asked) touches the new blocks. Asked of `quartus_sta` on the same
fit, the worst paths through them, at 10 ns:

| path | slack | about |
|---|---|---|
| out of the pointers, to the data memory address | +1.7 ns | 8 ns |
| into the pointers (from the instruction memory) | -2.0 ns | 12 ns |
| out of the `RPT` registers, to the instruction address | +1.6 ns | 8 ns |
| into the `RPT` turn counter | -14.7 ns | 25 ns |

The last one came from the counter counting a turn only without a jump
(`~pc_load`), which pulled the `JIZ` decision, made from the ALU, into the
counter. The contract already forbids a jump in the body, so the condition
was dropped (2026-10-26): the 11 forms still give the same bits; **its
path was not timed again**.

The -0.9 % is one seed each: `fmax.sh` warns that a fit can be off by
several per cent from one seed to another. Not yet shown either way.

### 6.6 `proc_rls` whole (measured, 2026-10-26)

The forms of 6.4 were run one at a time. `proc_rls` was then run whole:
`proto/rls_bench.py` (in the tag `archive/dirac-loops-proto`, beside
`bench.py`, whose loops it reuses) compiles the fixture with arrays of N, as
in section 1, and replaces every Dirac stretch, in `rls_update`, in the N
shifts and in the `out`, by the hand-written pointer and `RPT` loops; the
initialisation (`P # 1000|I|`, `w # |0⟩`), run once, stays unrolled. Both
versions are simulated for the 20 turns of the fixture, today's on
`SAPHO/`, the other on the prototype's copy, `F_SU2` in the update of P:

| N | output | words today -> pointers | clocks a turn today -> pointers |
|---|---|---|---|
| 4 | the same bits; today's also the golden | 339 -> 186 | 303 -> 349 (+15 %) |
| 16 | the same bits | 3951 -> 570 | 3663 -> 2989 (-18 %) |
| 32 | the same bits | 15 039 -> 1530 | 13 967 -> 10 541 (-25 %) |

Against the loops in software alone (3.5: 15 975 clocks at N = 16, 61 623 at
N = 32) the pointers are 5.3 and 5.8 times faster. Most of the 1530 words
left at N = 32 are the unrolled initialisation (over 1024 words) and the
test's own N copies of the read of `x`; a generator would make the
initialisation a loop as well (deduced). **Deduced:** the loss at N = 4
is the fixed cost of each stretch, the `SETP`s and the 7 instructions of
software loop a row of `|P|x⟩` and of the update of P; a second level of
`RPT`, or a pointer that walks a whole matrix, would remove it (6.7, 4).

### 6.7 Open, in order

1. Two more seeds of each (`proto/fit.sh` with `SEED=2`, `SEED=3`, a fit
   about 50 minutes, one at a time) with the counter fixed, and the path
   into the `RPT` counter timed again (`proto/sta.sh`).
2. Luciano decides: these instructions, and then decision 2 of section 5
   becomes a generator of its own for the Dirac loops.
3. In the repository, if yes: `SAPHO/core.v` and `processor.v` from the
   prototype (the diff to him first, as for every HDL change), the rows of
   `isa.tsv` and of the copies `check_isa.py` holds to it, asmcomp (the
   mnemonics `SETP` and `RPT`, the pointer syntax, the widened operand,
   `NPTR`/`RPT` from the program), the manual's instruction reference.
4. Open in the design: a second `RPT` level for the rows of a matrix;
   `comp` (two words an element); whether the pointers also serve
   ordinary C± arrays in loops (`for` with `a[k]`), which would speed up
   every program, not only the Dirac notation.
5. Other hardware for the loops, not built (deduced, 2026-10-26): a
   second level of `RPT`; a pointer with a programmable step (`+N`), so a
   column is walked as easily as a row (transpose, `⟨a|M|`, the outer
   product without swapping the loops); a pointer with row and column
   counters that walks a whole matrix in one loop; a decrement-and-jump
   counter in place of `RPT` (one clock a turn more, nothing in the fetch);
   pointers alone with the loops unrolled by 4 or 8. Left out: a float
   multiply-add in one clock, which would put the multiplier in series
   with the float adder, today's worst path.

### 6.8 Against the array unit

The two designs answer the same question, faster array code, at different
costs, and do not exclude each other (a parameter could choose):

| | pointers and `RPT` (this section) | array unit (array-unit.md) |
|---|---|---|
| logic | +98 ALMs (one seed) | about +920 ALMs, 1 RAM block, 1 DSP block |
| memory | today's | a second, dual-port memory for the arrays |
| clock | -0.9 % on one seed, not yet repeated | below what three seeds resolve |
| inner product, a word | 3 clocks | 1 clock |
| results | the bits of the loop in software | the MAC rounds once: other bits |
| compilers | a loop generator, two instructions | two address spaces, the S bit, more |

**Deduced:** over the pointers, the unit speeds up the N² parts of
`proc_rls` 2 to 3 times, not the tenfold it shows over loops in software;
any later decision on the unit is to be weighed against the pointers.

## 7. For the documentation (issue #40)

Done with step (b), on 2026-10-30: `docs/library.md`, "Vectors in Dirac
notation", says each point below, with the measured table of section 9
in place of that of 3.5.

The manual (`docs/library.md`, "Vectors in Dirac notation") replaces the
table of forms with the rules: what a chain between bars means (bra, ket,
matrix by place), that juxtaposition is the product and binds tighter than
`+` and looser than `*`, that the shapes must match and what the error says,
that a scalar in an expression is computed once before the element loop,
that int and float mix as in scalar expressions, when a statement uses the
temporary area and how to avoid it (the end-of-compilation summary of 3.4),
that `out()` does not take a matrix, and the forms the old table
listed as examples, including the declaration form `float v[N] # ...`,
which the manual never listed. It says that the code is a loop: its size
does not grow with the arrays, and it runs about four times slower per
element than the unrolled code of earlier versions, with the measured table
of 3.5. It says that a program which wrote `A # |a⟩⟨b|` or
`A # |B| - |a⟩⟨b|` with versions up to this one got the outer product
transposed, and that `a # |M|a⟩` and the int shift of an expression were
wrong, so that results computed with them need checking. The CHANGELOG says
the same, and that `golden.asm` of `dirac_assign` and `proc_rls` changed.

## 8. Where it stopped, and how to resume (2026-10-26)

Parked by Luciano on 2026-10-26, with `main` back to its own line of
releases. Nothing of it is in `main` but this note; the code is in tags:

- `archive/dirac-parser`: this note's history and step (a), the parser
  with shapes (commit 20112ab), every golden identical, twelve negative
  tests in `Compilers/CMMComp/NegTests/`. `git switch -c <branch>
  archive/dirac-parser` gives it back as a branch; `main` has moved since,
  so it is merged with `main` first.
- `archive/dirac-loops-proto`: the prototype of section 6 under
  `.smoke/dirac-proto/` (the SAPHO copy, `bench.py`, `rls_bench.py`, the
  fit scripts and their logs).

Where it stopped: section 6.7, item 1 (two more seeds of each fit, the
`RPT` counter's path timed again), then Luciano's decision on the
instructions of section 6 (item 2). Step (b) of section 5 follows from
that decision: a loop generator with the pointers, or loops in software.

**Resumed on 2026-10-29.** Luciano chose loops in software, with no
change to SAPHO: the pointers and `RPT` stay a measured option (section
6), not adopted. Step (a) was merged with `main` and brought in; step (b)
lowers the Dirac terms to loops with today's instructions, with the
temporary area of 3.4. Luciano asked (2026-10-29) that `proc_rls` be
checked against an RLS computed apart from YANC, not only against its
recorded output. The plan was to bring `rls_bench.py` (tag
`archive/dirac-loops-proto`) into the regression, but read again on
2026-10-30 it is not such a reference: it compares the unrolled code with
hand-written pointer loops, on the prototype's SAPHO, from paths outside
the repository. The reference is `Scripts/rls_ref.py` instead (section 9).
No check of `proc_rls`'s output can show the transposition of section 2:
its outer product is symmetric.

## 9. Step (b): loops in software (2026-10-30)

Luciano's words for it: the leanest code possible, with new virtual
instructions if needed, no hardware. Code in
`Compilers/CMMComp/Sources/dirac.c`; `ast.c` checks shapes and target and
hands the tree over.

**Two virtual instructions.** `LDI_V x k` and `STI_V x k` are `LDI` and
`STI` (opcodes 3 and 7) with a constant `k` added to the base, as `LOD_V`
is `LOD` with one: rows of `isa.tsv`, `ASMComp.l`, `pass1.l` and
`asm_share.c`, nothing in SAPHO. A loop counter runs from `-n` up to 0, so
the `JIZ` after its increment closes the loop with no comparison (the
`for` of C± tests with `LES` every turn), and the base sits at the end of
the array: `LDI_V a 4` with the counter at -4 reads `a[0]`.

**How an element is computed** (fact, `dirac.c`). For each element of the
result the Dirac tree becomes a scalar expression tree: `|a⟩` at row r is
`a[r]`, `|M|` is `M[r][c]`, `c X` is `X(r, c)*c` (the element first, the
scalar second, as the old generators), a product of inner size k is a sum
over k. The scalar walker (`ast_emit_expr`) emits that tree, with its
types, its mixing of int and float and its folding (`1|a⟩` has no
multiply); the array reads are new leaves, `EXPR_DIRAC_ELEM`, and the sums
`EXPR_DIRAC_SUM`. Further:

- a sum keeps its partial result on the stack; its first term is taken out
  of the loop, and a cell at index 0 is the array's own name, a memory
  operand: `⟨a|b⟩` starts `LOD a; F_MLT b`. A sum of 2 terms has no loop;
- an index of a matrix is a sum of loop variables, never a multiply: a
  loop's counter steps by the stride its index is used with most deeply
  (the row length for the rows of `|M|x⟩`), and another stride gets a
  variable stepped with the counter (requirement 3 of 3.4);
- a matrix expression that only adds and scales matrices of the target's
  shape runs over one flat index;
- what does not depend on the inner loop is computed at the top of the
  outer one (`K[r]` in `|K⟩⟨Px|`), and a scalar that is not a literal or a
  variable before the loops (requirement 2), converted once when the
  elements are float and it is int;
- the target read at other cells than the one written goes through
  `__dirac_tmp` and is copied back (requirement 4); a product inside a
  chain that does not depend on every loop around it is computed into
  `__dirac_tmp` first, and a chain closed by a ket is read from the right,
  `|A|B|x⟩` as `A (B x)` (requirement 5). One area for the program, as
  large as the largest statement needs, declared after `#PRNAME` at the
  end of the compilation (requirement 6);
- the shift register stays `exec_shift`, unrolled (2 words an element);
  its int case of section 2 is added.

Instructions per element in the loop body, read in the assembly of
`proc_rls` (fact), against the unrolled code and the `for` loops of 3.5:

| form | unrolled | `for` loops (3.5) | loops of step (b) |
|---|---|---|---|
| `⟨a\|b⟩` | 3 | 12 | 10 |
| a row of `\|P\|x⟩` | 3 | 14 | 11 |
| `\|P\| - \|K⟩⟨Px\|` | 5 | 20 | 13 |
| `1.010101\|P\|` | 3 | 14 | 9 |
| `g\|Px⟩` | 3 | | 9 |
| `\|w⟩ + e\|K⟩` | 4 | | 12 |
| `out(0, 1000.0\|w⟩)` | 4 | 10 | 9 |
| `\|0⟩`, the zeros of `c\|I\|` | 1 | | 7 |

Five of each are the loop (`LOD k; ADD 1; SET k; JIZ; JMP`, or `P_LOD k`
first in a sum), and a store takes two (`PSH` of the index, `STI_V`).

**Measured** with the method of 1.1 (`proc_rls` with arrays of N, Icarus,
clocks between the first `OUT` of two turns), `main` at 515de8f (the
unrolled code, whose numbers are those of 1.1) against this branch:

| N | words, unrolled | words, loops | clocks a turn, unrolled | clocks a turn, loops |
|---|---|---|---|---|
| 4 | 335 | 235 | 307 | 855 |
| 8 | 1091 | 343 | 1011 | 2811 |
| 16 | 3947 | 751 | 3667 | 10 083 |
| 32 | 15 035 | 2335 | 13 971 | 38 067 |

The loops follow clocks = 35N² + 69N + 19 (fitted on 4, 8 and 16, exact at
32) and words = 2N² + 3N + 191, whose 2N² are the test's own N unrolled
shifts. The two give the same output over the five turns both simulate
at every N. Per turn the loops take 2.7 to
2.8 times the unrolled code's clocks, and 0.62 to 0.70 times those of the
`for` loops of 3.5 (58N²).

**Tests.** `dirac_shapes` (new): every element of 49 Dirac statements, int and
float, the outer product not square, `|B| ± |a⟩⟨b|` with `B` not
symmetric, `a # |M|a⟩` and `C # |M||C|` through the temporary area,
chains, `⟨a|M|`, the int shift, `|in(p)⟩`, `out` of a vector, 160 values;
its golden is written by `Scripts/dirac_ref.py`, the same statements in
Python on integers the float format holds exactly, and the regression
checks that it still is. `Scripts/rls_ref.py` computes the RLS of
`proc_rls` in double precision from its input files: the recorded output
is that filter's, with no difference in any of the 80 words. Measured by
changing the filter there: without the update of P the largest difference
is 4348, with P started at 10 instead of 1000 it is 653, but a forgetting
factor of 1 gives 0, and so does the transposition, so this checks the
filter as a whole and `dirac_shapes` the details. `cases.tsv` has 176
combinations. `proc_rls/golden_sim` is unchanged; `dirac_assign`, which
writes 1 in a loop forever, writes 195 of them in the regression's
100 000 clocks instead of 812 (the same values).

**Corrected after the audit of 2026-10-30.** Three defects of this step,
found by reading `dirac.c` and checked by compiling, each one now in
`dirac_shapes`, which the code before the correction fails (x came out
1838 instead of 1847):

- a Dirac number inside a scalar expression whose chain went through the
  temporary area overwrote the value the walker held in the accumulator
  (`x = x*3.0 + ⟨a|M|M|M|e⟩` gave 0 + the number): that value now goes to
  a variable before the first instruction of the number;
- the product of a chain went to `__dirac_tmp` before the parts computed
  before the loops, so a function called from a coefficient that has a
  Dirac statement of its own overwrote the block before it was read: the
  parts now go first, and the statement is built again reading them;
- the compiler's variables had the same names in every function, so that
  call also overwrote the caller's coefficients: they now carry the
  function's name.

Also: the left operand is built before the right one (C left the order to
the host compiler, and with it the order of two `in(p)` and the
numbering of the variables). The compiler's own tables grow as a statement
needs: before, a statement stopped at 64 loops (63 sums such as `|M|a⟩`
measured), 64 parts computed before the loops or 8 row lengths read by
one loop (7 matrices of different widths measured), sizes picked when the
code was written, with no reason in the hardware, and the sets of loops
were 64-bit masks; they are now questions to the tree (`dep_on`).

**Step (c), 2026-10-30.** The eleven generators of 1.2 (`exec_vtv` to
`exec_vout`, 840 lines of `stdlib.c`) and the twenty messages only they used
are gone; nothing reached them since step (b) (`Scripts/cmm_cov.sh`, issue
#69: no test and no grammar rule did). The shift register `exec_shift`
stays.

**Open.** `|I|` inside a larger expression, and `|I|` or `|0⟩` inside
a chain (a syntax error today). `comp` (decision 5). An `int` counter of
an index past the word: at `#NUBITS 8` a flat loop over more than 128
cells overflows (deduced, as a `for` would; not checked). Unrolling a loop
of very few turns, which decision 6 left out: at N = 2 the loop costs more
than the unrolled code in words and in clocks (deduced).
