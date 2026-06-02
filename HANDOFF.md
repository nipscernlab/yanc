# Handoff — branch `claude/windows-batch-generic-setup-IsUtm`

Documento de continuação. Tudo aqui é relativo à `main`. Escrito ao fim de uma
sessão em container **Linux**; o que falta é majoritariamente **validar no
Windows** (sua máquina).

---

## TL;DR — estado atual

- **Regress 71/71 verde no Linux**, com ambiente limpo (nenhuma variável setada,
  nenhum caminho hardcoded). `bash Scripts/regress.sh` → `===== 71 passed, 0 failed =====`.
- Os **runners** foram refatorados, renomeados e enxugados; os `.sh` foram
  validados ponta a ponta (iverilog **e** verilator). Os **`.bat` NÃO rodaram
  aqui** (não há cmd no Linux) — precisam de uma passada no Windows.
- Achei e corrigi **4 fragilidades/bugs** que faziam o regress falhar fora da
  máquina do Aurora (detalhes em §Bugs). Nenhum foi causado pelo trabalho dos
  runners — eram latentes, mascarados pelos caminhos Windows-only.
- **⚠️ NADA deste branch foi testado no Windows ainda.** Toda a validação foi no
  Linux. O lado Windows inteiro — `setup.bat`, os dois `.bat`, o build dos
  compiladores com `-Werror` sob MSYS2, e o `regress.sh` sob MSYS2 — está
  **pendente de verificação**. Esse é o trabalho principal que sobra. Checklist
  em §3.1.
- **Risco específico:** o `regress.sh` deixou de hardcodar o caminho do iverilog
  do Aurora e passou a resolver via `Scripts/env.sh`. No Windows isso exige
  iverilog/vvp/verilator no PATH do MSYS2 **ou** ter rodado `bash Scripts/setup.sh`.

---

## 1. O que mudou desde a `main`

### 1.1 Runners da raiz (o foco da sessão)

Eram 8 arquivos (`go_proc{,_vl}.{sh,bat}`, `go_proj{,_vl}.{sh,bat}`). Agora são 4:

| Antes | Agora |
| --- | --- |
| `go_proc.{sh,bat}` + `go_proc_vl.{sh,bat}` | `single_proc.{sh,bat}` |
| `go_proj.{sh,bat}` + `go_proj_vl.{sh,bat}` | `multi_proc.{sh,bat}` |

Mudanças aplicadas, em ordem:
1. **Fusão dos simuladores** num só script via flag `--sim iverilog|verilator`
   (default iverilog). Aceita também `--sim=...`, `vl`, `icarus`, `--help`.
2. **Enxugados pro caminho `.cmm → onda`**: removido o mirror `saphoComponents`
   (cópias de HDL/Macros/Scripts/bin). HDL, macros e binários agora são **lidos
   in-place do repo**; só se escreve em `Teste/` (gitignored).
3. **Removida a opcionalidade de tb/gtkw**: sempre usa o testbench gerado pelo
   asm e o layout gerado pelo `gen_gtkw` (tiramos `TB`/`GTKW`/`SIMU_DIR` e os
   `if`s de override). `multi_proc` mantém `TB=top_level_tb` (é o top obrigatório).
4. **Renomeados** `go_proc → single_proc`, `go_proj → multi_proc` (e todas as
   referências: README, setup.{sh,bat}, ci.yml, regress.sh).
5. **Headers descritivos** explicando que são exemplos de pipeline completo
   (`.cmm → .v sintetizável p/ FPGA → simular → gtkwave`).
6. **Cópia mínima**: copia só o(s) `.cmm` (e, no multi_proc, o `TopLevel/`),
   não os projetos inteiros (sem `golden_sim/`, `golden.asm`).

**Validação Linux (gtkwave stubado, iverilog+verilator reais):**

| | Icarus | Verilator |
| --- | --- | --- |
| single_proc | 2576 B ✓ | 94218 B ✓ |
| multi_proc | 8,46 MB ✓ | 417951 B ✓ |

### 1.2 Base já existente no branch (antes desta sessão)

Commits `d43cc27`..`ca3e652`: cross-platform setup (`setup.{sh,bat}`,
`env.{sh,bat}`), `Makefile` como fonte única do build, re-ativação do `-Werror`
no CI, e o sweep de `-Wall` nos fontes dos compiladores. **Importante:** este
branch *tocou quase todos os fontes dos compiladores* (CMM/ASM/APP/CPP) nesse
cleanup. O regress confirma que esses fontes estão corretos (fase CMM 20/20).

### 1.3 `Scripts/regress.sh` (de-fragilizado nesta sessão)

- Resolve iverilog/vvp/verilator via `. Scripts/env.sh` (cache `tools.local.sh`
  + fallback PATH), **em vez** de hardcodar `/c/nipscern/Aurora/.../iverilog.exe`,
  `VERILATOR_ROOT=C:/packs/...` e `export PATH=C:/packs/...`.
- Adicionado `-lm` nas linhas de link do próprio build (alinhando ao Makefile).
- Passo verilator dos testes CPP: roda **nativo em bash** quando não há
  `powershell.exe` (Linux); mantém o helper PowerShell no Windows.

### 1.4 `Compilers/CPPComp/Sources/cpppp.c` (bug fix)

Corrigido um buffer overflow no `path_canon` (ver §Bugs).

---

## 2. Bugs encontrados

Todos **pré-existentes** (idênticos na `main`) e **invisíveis no Windows** —
mascarados por ramos `#ifdef _WIN32`, PowerShell ou caminhos hardcoded do
Aurora. Só apareceram porque este container Linux exercitou esses caminhos pela
1ª vez.

| # | Onde | Causa | Status |
| --- | --- | --- | --- |
| 1 | `regress.sh` build | `gcc` próprio sem `-lm` (divergiu do Makefile) → `undefined reference to pow` | **corrigido** (`-lm`) |
| 2 | `regress.sh` tools | caminho do iverilog/vvp hardcoded p/ instalação do Aurora | **corrigido** (via `env.sh`) |
| 3 | `cpppp.c:50` `path_canon` | `realpath(in, tmp[2048])` — `realpath` precisa de PATH_MAX (4096); só o ramo POSIX toca isso, Windows usa `_fullpath`. glibc FORTIFY aborta | **corrigido** (`realpath(in, NULL)`) |
| 4 | `regress.sh` verilator CPP | passo via `powershell.exe` + `C:/packs/...`; não existe no Linux | **corrigido** (ramo bash nativo) |

Detalhe do #3 (o mais sério — corrupção de memória real): `path_canon` passava
um `char tmp[2048]` pro `realpath`, que pode escrever até PATH_MAX. Provei que é
pré-existente compilando o `cpppp.c` da `main` aqui e reproduzindo o mesmo abort.
Fix: usar `realpath(in, NULL)` (glibc aloca o tamanho certo) e mover o `tmp[]`
pra dentro do `#ifdef _WIN32` (senão sobra variável não-usada no `-Werror`).

---

## 3. O que ainda falta pra deixar o regress 100% correto

### 3.1 CRÍTICO — validar TODO o fluxo Windows (nada foi testado lá ainda)
Tudo abaixo é Windows-virgem. Checklist, do mais fundamental ao mais específico:

1. **`Scripts\setup.bat`** — roda do zero? Acha/instala o toolchain (gcc/bison/
   flex via pacman), produz os binários em `bin\`, acha iverilog/verilator/
   GTKWave e grava `Scripts\tools.local.bat`. Esse é o alicerce de tudo.
2. **Build dos compiladores com `-Werror` sob MSYS2** — o branch fez um sweep de
   `-Wall` em quase todos os fontes (CMM/ASM/APP/CPP) e re-ligou `-Werror`.
   Confirme `make` limpo no MSYS2 (no Linux passa). Em especial o `cpppp.c`, que
   eu mexi: no Windows usa o ramo `_fullpath` (comportamento inalterado), mas
   movi a declaração do `tmp` — confirme que compila sem warning.
3. **`single_proc.bat` e `multi_proc.bat`** — os 4 cenários (cada um com e sem
   `--sim verilator`). São espelhos mecânicos dos `.sh` validados, mas **nunca
   rodaram em cmd**. Pontos de atenção do cmd que vale conferir: o parsing de
   `--sim`, os blocos `goto`/`if`/`setlocal enabledelayedexpansion` (no
   multi_proc), e a abertura da onda no GTKWave.
4. **`regress.sh` sob MSYS2** — agora resolve ferramentas via `env.sh`. Garanta
   iverilog/vvp/verilator no PATH do MSYS2 **ou** rode `bash Scripts/setup.sh`
   antes (gera `Scripts/tools.local.sh` — atenção: `setup.bat` gera
   `tools.local.bat`, que o `env.sh` **bash não lê**). Se os sims falharem, é
   quase certo que é isso.
5. **Passo verilator do regress no Windows** — lá o `powershell.exe` existe,
   então segue usando o `run_verilator_step.ps1` (inalterado). Mas esse `.ps1`
   tem `C:/packs/msys64/...` cravado (§3.4) — se seu MSYS2 não estiver em
   `C:/packs/msys64`, ajuste.

### 3.2 Fragilidade que ainda resta — build duplicado no `regress.sh`
O `regress.sh` ainda **refaz o build com `gcc` próprio** (uma linha por binário,
lines ~166-201), em paralelo ao `Makefile`. São duas descrições do mesmo build
que divergem — foi exatamente assim que o `-lm` sumiu só no regress. O `-lm` que
adicionei é band-aid. O fix de raiz: o regress **chamar o `Makefile`** (fonte
única). NÃO foi feito (mexe no build e não dá pra testar no Windows daqui).

**Receita pronta (validar no Linux E no Windows depois):**

A `.gitignore` já cobre `lex.yy.c`/`y.tab.c`/`*.exe`/`/.smoke/`, então o `make`
não suja a árvore. Substituir o bloco de build (`if [ "$SKIP_BUILD" -eq 0 ]`)
por algo como:

```sh
# sufixo .exe como o Makefile (ifeq $(OS),Windows_NT)
[ "${OS:-}" = "Windows_NT" ] && EXE=.exe || EXE=
rm -rf "$BIN_DIR"
# -Werror só em cmm/app/asm; cpppp e cppcomp como o Makefile (sem -Werror);
# CPP_DEFS (-DCFG_*) injetado via CFLAGS pro cppcomp.
make -C "$ROOT" BIN="$BIN_DIR" CFLAGS="-O2 -Wall -Werror" cmmcomp appcomp asmcomp || exit 1
make -C "$ROOT" BIN="$BIN_DIR" CFLAGS="-O2"               cpppp             || exit 1
make -C "$ROOT" BIN="$BIN_DIR" CFLAGS="-O2 $CPP_DEFS"     cppcomp           || exit 1
```

E trocar os nomes dos binários (lines 84-88) de `.exe` cravado para o sufixo:
```sh
CMMCOMP="$BIN_DIR/cmmcomp$EXE"   # idem cpppp/cppcomp/appcomp/asmcomp
```
Por que 3 invocações: cada `make` usa um `CFLAGS`; o `-Werror` é seletivo e o
`cppcomp` precisa dos `-DCFG_*`. Depois disso, o `-lm` band-aid pode sair (o
Makefile já tem `LDLIBS=-lm`). Rodar `bash Scripts/regress.sh` → deve seguir
71/71.

### 3.3 `.bat` dos runners — testar no Windows
`single_proc.bat` e `multi_proc.bat` são espelhos mecânicos dos `.sh` validados,
mas **não rodaram aqui**. Rode os dois (com e sem `--sim verilator`) e confirme
que abrem a onda no GTKWave.

### 3.4 `run_verilator_step.ps1` — ainda tem `C:/packs/...` cravado
O helper PowerShell hardcoda `C:/packs/msys64/...` (verilator, VERILATOR_ROOT,
TMP). Se seu MSYS2 estiver em `C:/msys64`, ajuste ou de-fragilize (resolver o
verilator do PATH como o resto). Pré-existente, não toquei.

---

## 4. Como continuar da sua máquina

```sh
# 0. pegar o branch
git fetch origin claude/windows-batch-generic-setup-IsUtm
git checkout claude/windows-batch-generic-setup-IsUtm

# 1. (Windows/MSYS2) garantir que o env.sh acha as ferramentas
bash Scripts/setup.sh          # gera Scripts/tools.local.sh; ou tenha as tools no PATH

# 2. rodar o regress completo
bash Scripts/regress.sh        # esperado: 71 passed, 0 failed
#   --skip-build   reusa .smoke/bin (mais rápido em re-runs)
#   --no-sim       pula simulação (só compara .asm/.v dos goldens)
#   --cmm-only / --cpp-only   roda só uma fase

# 3. testar os runners
./single_proc.sh                    # FFT, Icarus
./single_proc.sh --sim verilator
./multi_proc.sh                     # DTW, Icarus
./multi_proc.sh --sim verilator
#   (no Windows: single_proc.bat / multi_proc.bat, idem)
```

Se algo falhar no Windows, o suspeito #1 é a resolução de ferramentas (§3.1).
Pra depurar, veja o que o `env.sh` resolveu:
```sh
. Scripts/env.sh; echo "iverilog=$IVERILOG vvp=$VVP verilator=$VERILATOR"
```

---

## 5. Pontos em aberto (ofertas que ficaram pendentes)

- **Traduzir os headers dos runners pra português** (hoje estão em inglês, pra
  casar com o resto do repo).
- **Unificar o build do `regress.sh` via Makefile** (§3.2) — elimina de vez a
  duplicação que causou o bug do `-lm`.
- **De-fragilizar o `run_verilator_step.ps1`** (§3.4).
- Decidir se este `HANDOFF.md` fica no repo ou é descartado depois.

---

## 6. Commits desta sessão (mais recentes primeiro)

```
a161478 regress.sh: run the CPP Verilator step natively when PowerShell is absent
0f3898f cpppp: fix realpath buffer overflow in path_canon (POSIX branch)
06b03a3 regress.sh: resolve tools via env.sh, link with -lm (kill hardcoded paths)
0ff7998 multi_proc: copy only the inputs, not the whole projects
7d1926e multi_proc header: state the project was tested on an FPGA
7b328a9 Document multi_proc (top-level + 2 processors) and clarify single_proc sim
2caaeb8 single_proc: copy only the .cmm, not the whole project
6cddf8f Document single_proc as the end-to-end pipeline example
7c74e9b Rename the runners: go_proc -> single_proc, go_proj -> multi_proc
2c6cc15 Drop the testbench / layout overrides from the go_* runners
f8ab819 Slim the go_* runners down to the .cmm -> waveform path
c537ee8 Merge the per-simulator go_* scripts behind a --sim flag
```
(abaixo destes, a base do branch: Makefile, setup cross-platform, -Werror.)
