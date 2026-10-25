# Dirac notation as expressions, computed in loops

Status: design, 2026-10-25. No code yet. Decision (Luciano, 2026-10-25): the
Dirac notation of C± stops being a closed set of grammar rules, one per
form, each with a generator that unrolls the computation, and becomes
expressions in the parser: every operator a node of the tree, with shape
rules (a vector of N, an N×M matrix, a bra or a ket) and a compile error
when the shapes do not match. The code is generated as loops in software.
Choosing, later, between the software loop and the array unit of PR #19
([array-unit.md](https://github.com/nipscernlab/yanc/pull/19), branch
`lucianomaf/array-unit-design`) is separate work; nothing here depends on
that unit.

Marks: **fact** (read in the code, with file and line), **measured** (run on
this branch, method given), **deduced** (reasoned, not run).

## 1. Why the unrolled form cannot stay

### 1.1 Program size and time grow with N² (measured)

Every Dirac form is unrolled at compile time: one group of instructions per
element, no loop. To see what that costs, `proc_rls` (the RLS fixture,
`Compilers/CMMComp/Tests/proc_rls`) was compiled with every `[4]` replaced by
`[N]` and the read of `x` (`x # fin(0)*0.001 -> |x⟩;`) repeated N times, as
the fixture repeats it 4 times. Words: `n_ins` in `app_log.txt`. Clocks: a
probe on the core counts the clocks between the first `OUT` of two loop turns
(an `OUT` that follows at least 8 clocks without one). Branch
`lucianomaf/dirac-expressions` at 46f4067, built with
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

There are no Dirac expressions in the parser. Each form is a statement rule
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

The full survey is in issue #18. Measured on this branch with small
programs compiled and simulated (2026-10-25):

| what | expected | today | where |
|---|---|---|---|
| `A # \|a⟩⟨b\|`, a = (1, 2), b = (10, 30) | 10 30 20 60 | 10 20 30 60 | `exec_vvt` writes `N*j+i` (`stdlib.c` 3046) |
| `A # \|B\| - \|a⟩⟨b\|`, B = (100 200; 300 400) | 90 170 280 340 | 90 180 270 340 | `exec_Mmvvt` reads and writes `N*j+i` (3124, 3135): it computes \|B\| − \|b⟩⟨a\| |
| `a # \|M\|a⟩`, M = (0 1; 1 0), a = (1, 2) | 2 1 | 2 2 | `exec_Mv` writes `a[0]` before the second row reads it |
| `int a[3]; a # k+1 -> \|a⟩;`, k = 5 | 6 0 0 | 0 0 0 | `exec_shift` has no case for an int vector and an int in the accumulator (3505-3559): it emits `LOD k; ADD 1` and nothing else, without an error |

The fixtures miss the first three: in `proc_rls` the outer product is
K Pxᵀ with K = g Px, symmetric up to rounding, and `|P| - |K⟩⟨Px|` writes P
in place at the same element; `dirac_assign` never outputs a matrix and its
`a # |B|a⟩` has B = I.

Limits, from the code (fact) unless marked:

- no `comp` in any form (`MSG_ERR_NOT_IMPL_COMPLEX` in every generator);
- the scalar must have the vector's type: no int/float mix
  (`MSG_ERR_VARS_SAME_TYPE`);
- with the scalar 1, every form still multiplies: `out(0, 1|a⟩)` is
  `LOD_V`, `MLT 1`, `OUT` per word, and the multiplier is synthesized for it;
  `out(p, |a⟩)` without a scalar is a syntax error (TODO item 31);
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
`isar` set) is the token `DBAR`; any other `|` is the bar it is today
(bitwise or, or the bar that closes a matrix or a bra). An array's name
alone is not an operand today (measured: `int y = 3 | a;` with `int a[4]`
stops with "where's the array index for the variable 'a'?"), so no program
that compiles today changes meaning. `|in(p)⟩` is lexed whole, like `|0⟩` today,
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
| `MSG_INFO_DIRAC_TEMP` | line %d: %s is read and written with a different index: the result goes through a temporary of %d words |

with a shape written as "a vector of 4", "a row of 4", "a 4x3 matrix". The
element types follow the scalar rules of `typecheck_expr` (int, float,
comp), so the mix of int and float is allowed where `+` and `*` allow it
today. With `comp`, a bra conjugates.

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
   `* 1` removes `1|a⟩`'s multiply (TODO item 31).
3. **No `MLT` for an index.** The 2D access of the general array code
   computes `r*N + c` with `MLT` (`LOD r; MLT 4; ADD c; LDI P`, measured in
   3.5), which synthesizes the integer multiplier in a program that may use
   only floats: measured, the loop version of `proc_rls` turns on `MLT` and
   `SF_MLT` in its processor, the Dirac version only `F_MLT`. The lowering
   keeps a running row offset (an `ADD` of N per row), and an element-wise
   matrix operation (`c|A|`, `|A| - |B|`, the zero) runs over one flat index
   of N·M. The area of the extra multiplier was not measured.
4. **Aliasing.** When the target is read with a different index from the
   one written (`a # |M|a⟩`, `P # |P||Q|`, a transpose), the result goes to
   a compiler temporary array of the target's size and is copied at the end,
   with `MSG_INFO_DIRAC_TEMP`. Read at the same index (`P # |P| - |K⟩⟨Px|`)
   needs nothing. Data memory is static, so the temporary is one more array
   of the program (deduced: N words, or N·M).
5. **Chains are grouped to keep the work at N²**: `|A|B|x⟩` is `A (B x)`,
   with `B x` materialised in a temporary, not `(A B) x`.
6. **Names.** Loop counters, accumulators and temporaries start with `__`
   (design note asm-names.md, rule 2), one set per nesting depth, shared by
   every Dirac statement of the program: C± has no recursion and the scalar
   subtrees are computed before the loops, so no two uses overlap (deduced).

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
  PR #19 is the way to win that back; this work does not try to.
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
  the processor no multiplier (TODO item 31);
- int with float: `float v[3] # k|a⟩` with `int k`.

And a fixture of compile errors, one per message of 3.2, each checked for
its line and its text in both languages.

## 5. Open decisions

- G3's lexer rule, and the shape names in the messages.
- Whether `out(p, X)` accepts a matrix (row by row) or refuses it.
- Whether a temporary for aliasing is silent, an info line (proposed) or an
  error that asks the user to write the temporary.
- Whether to keep unrolling for very small N (L3): at N = 2, `out(0, |a⟩)`
  unrolled is 4 words and 4 clocks, the loop about 10 words and 20 clocks
  (deduced). Proposed: no, one code path first, measured later.
- `comp` now or after: the scalar walker handles `comp`; what is missing is
  the conjugation of a bra and a fixture.

## 6. For the documentation (item 21 in TODO.md)

The manual (`docs/library.md`, "Vectors in Dirac notation") replaces the
table of forms with the rules: what a chain between bars means (bra, ket,
matrix by place), that juxtaposition is the product and binds tighter than
`+` and looser than `*`, that the shapes must match and what the error says,
that a scalar in an expression is computed once before the element loop,
that int and float mix as in scalar expressions, and the forms the old table
listed as examples, including the declaration form `float v[N] # ...`,
which the manual never listed. It says that the code is a loop: its size
does not grow with the arrays, and it runs about four times slower per
element than the unrolled code of earlier versions, with the measured table
of 3.5. It says that a program which wrote `A # |a⟩⟨b|` or
`A # |B| - |a⟩⟨b|` with versions up to this one got the outer product
transposed, and that `a # |M|a⟩` and the int shift of an expression were
wrong, so that results computed with them need checking. The CHANGELOG says
the same, and that `golden.asm` of `dirac_assign` and `proc_rls` changed.
