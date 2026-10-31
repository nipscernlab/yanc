#!/usr/bin/env bash
# Scripts/regress.sh - single regression suite for the whole repo.
#
# Runs both compiler pipelines off a shared set of binaries:
#
#   1. CMM phase: for every Compilers/CMMComp/Tests/<prname>/<prname>.cmm
#      run cmmcomp and compare the produced .asm against
#      Compilers/CMMComp/Tests/<prname>/golden.asm. For the sim-enabled subset also
#      run asmcomp + iverilog + vvp and compare output_*.txt
#      against Compilers/CMMComp/Tests/<prname>/golden_sim/. Plus a project pass for
#      the multi-proc DTW example (Compilers/CMMComp/Tests/DTW/TopLevel/).
#
#   2. CPP phase: for every Compilers/CPPComp/Tests/testN/testN.cpp run
#      cppcomp -> asmcomp -> iverilog (or Verilator
#      if testN/testN.in is present) and compare the resulting
#      output_0.txt against Compilers/CPPComp/Tests/testN/golden.txt.
#      Every test is a folder holding the source and its own files (array
#      inits, input_N.txt); the tools write everything they generate to one
#      output folder, <work>/<test>/<prname>/ (the default ./<prname> of the
#      user's flow). An optional testN/warnings.txt lists
#      the cppcomp warnings the test expects, one substring per line: each
#      must appear, and no other warning may. An optional hardware.txt (in
#      both phases) holds the generated processor's parameters: see hw_check.
#      Every Icarus run, in both phases, also fails when a stack reached its
#      depth (fl_full in the VCD): see stack_check.
#
#   3. ISA table: hold the hand-written copies of the instruction set
#      (ASMComp.l, core.v) to Compilers/common/isa.tsv, so the four places
#      that spell the ISA out cannot drift apart silently. See
#      Scripts/check_isa.py. In the same step, Scripts/check_changelog.py
#      holds CHANGELOG.md's link lines to its version headings, and
#      Scripts/dirac_cases.py runs every combination of the Dirac notation
#      in Compilers/CMMComp/DiracCases/cases.tsv through cmmcomp, and
#      Scripts/dirac_ref.py and Scripts/rls_ref.py hold the sim goldens of
#      dirac_shapes and proc_rls to values computed apart from YANC.
#      Scripts/cmm_cov.sh: no more of cmmcomp's assembly-emitting calls may
#      go unreached by the tests, or unchecked against an independent
#      reference, than Compilers/CMMComp/Tests/cov_baseline.txt allows.
#      Scripts/cov_tests.py --check: the generated tests are what it computes.
#      Scripts/stack_balance.py: no statement of a test leaves a word on the
#      data stack.
#
#   3b. Float and complex conditions: Scripts/fcond runs the same cases on
#      the host C/C++ compiler and on both compilers (three float formats,
#      three #FROUND levels) and compares C's answers.
#
#   4. CMM negative phase: for every fixture in Compilers/CMMComp/NegTests/
#      (listed in NegTests/manifest.txt) run cmmcomp and assert it REJECTS the
#      malformed program with a clean non-zero exit and the expected diagnostic
#      -- never a crash, never a silent accept. Error-path coverage the golden
#      phases don't give. (Runs with the CMM phase; skipped by --cpp-only.)
#      The CPP negative phase does the same for cppcomp with
#      Compilers/CPPComp/NegTests/manifest.txt (skipped by --cmm-only).
#
# Build phase builds the 4 binaries it runs once (cmmcomp, cppcomp, asmcomp, yanc), all
# phases reuse them.
#
# Usage (from repo root, in msys2/git-bash on Windows):
#   Scripts/regress.sh                check against goldens (exit 0 = pass)
#   Scripts/regress.sh --update       regenerate goldens (review diff!)
#   Scripts/regress.sh --update-size  ratchet down Compilers/CMMComp/Tests/size_baseline.txt
#   Scripts/regress.sh --skip-build   reuse binaries already in .smoke/bin
#   Scripts/regress.sh --no-sim       skip every simulation step
#   Scripts/regress.sh --cmm-only     skip the CPP phase
#   Scripts/regress.sh --cpp-only     skip the CMM phase

set -uo pipefail

# Force TMP/TEMP to a writable Windows path. mingw g++ -- invoked deep
# inside Verilator's makefile (verilator -> make -> g++) -- needs a
# Windows-style temp dir for its intermediate .s files; otherwise it
# silently falls back to C:\WINDOWS\ where unprivileged users can't write
# ("Cannot create temporary file in C:\WINDOWS\: Permission denied").
# Setting all three unconditionally is the only thing that survives the
# PowerShell -> bash -> make -> g++ env-inheritance chain. The dir is MSYS2's
# own /tmp in its Windows form (cygpath -m), wherever MSYS2 is installed;
# off Windows (no cygpath) TMP is left alone. The dir is made through /tmp:
# MSYS mkdir reads the bare drive prefix "C:" as a *relative* directory name
# and would leave a junk "./C:" folder in the repo root.
if command -v cygpath >/dev/null 2>&1; then
    mkdir -p /tmp
    export TMP="$(cygpath -m /tmp)"
    export TEMP="$TMP"
    export TMPDIR="$TMP"
fi

UPDATE=0
UPDATE_SIZE=0
SKIP_BUILD=0
NO_SIM=0
CMM_ONLY=0
CPP_ONLY=0
for arg in "$@"; do
    case "$arg" in
        --update)       UPDATE=1 ;;
        --update-size)  UPDATE_SIZE=1 ;;
        --skip-build)   SKIP_BUILD=1 ;;
        --no-sim)       NO_SIM=1 ;;
        --cmm-only)     CMM_ONLY=1 ;;
        --cpp-only)     CPP_ONLY=1 ;;
        -h|--help)
            sed -n '1,/^$/p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "regress: unknown option: $arg" >&2
            exit 2
            ;;
    esac
done

# Resolve repo root from the script's own location so it works no matter
# where you invoke it from.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export YANC_LANG=en               # the expected messages are the English ones
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SCRATCH="$ROOT/.smoke"
BIN_DIR="$SCRATCH/bin"
WORK_DIR="$SCRATCH/work"          # CMM phase scratch
WORK_CPP="$SCRATCH/work_cpp"      # CPP phase scratch
TMP_DIR="$SCRATCH/tmp"            # CMM phase tmps

CMMCOMP="$BIN_DIR/cmmcomp.exe"
CPPC="$BIN_DIR/cppcomp.exe"
ASMCOMP="$BIN_DIR/asmcomp.exe"
YANC="$BIN_DIR/yanc.exe"
HDL="$ROOT/SAPHO"
CMM_ROOT="$ROOT/Compilers/CMMComp"
SIZE_BASELINE_FILE="$CMM_ROOT/Tests/size_baseline.txt"

CPP_ROOT="$ROOT/Compilers/CPPComp"
SIMMAIN="$CPP_ROOT/Tests/Verilator/sim_main.cpp"

# CPPComp build flags (target params burned into cppcomp.exe). Override
# via env when building a non-default-target binary.
# The float format follows from CFG_NUBITS and asmcomp sizes the stacks.
: "${CFG_NUBITS:=32}"; : "${CFG_NUGAIN:=128}"
: "${CFG_NUIOIN:=1}";  : "${CFG_NUIOOU:=1}"
: "${CFG_HEAPSZ:=2048}"
CPP_DEFS="-DCFG_NUBITS=$CFG_NUBITS -DCFG_NUGAIN=$CFG_NUGAIN \
          -DCFG_NUIOIN=$CFG_NUIOIN -DCFG_NUIOOU=$CFG_NUIOOU \
          -DCFG_HEAPSZ=$CFG_HEAPSZ"

# Resolve iverilog / vvp / verilator the same way the runners do: Scripts/env.sh
# loads the setup.sh cache (tools.local.sh) and falls back to a PATH lookup, so
# there are no machine-specific hardcoded tool paths here. A pre-set env var
# (e.g. CI, or a non-default Verilator) still wins.
. "$ROOT/Scripts/env.sh"

# load the size ratchet (prname -> num_ins). Lines starting with '#' are skipped.
declare -A SIZE_BASELINE
if [ -f "$SIZE_BASELINE_FILE" ]; then
    while IFS= read -r line || [ -n "$line" ]; do
        [ -z "$line" ] && continue
        [ "${line:0:1}" = "#" ] && continue
        name=$(echo "$line" | awk '{print $1}')
        val=$(echo "$line"  | awk '{print $2}')
        [ -n "$name" ] && [ -n "$val" ] && SIZE_BASELINE[$name]="$val"
    done < "$SIZE_BASELINE_FILE"
fi
declare -A SIZE_CURRENT

# Up to two retries for a simulation that DIED. On a machine short of memory
# vvp can die mid-run with no message, on a different heavy float test each
# run; the same .vvp re-run then passes (twice in a row was seen once).
# Measured in 2026-09: nothing on stderr, a truncated VCD, and 1 GB free of
# 15.5 GB with one simulation 8x slower than usual -- paging, not YANC or
# Icarus, and not the waveform writer (without $dumpvars the time is the same). A simulation that exits
# non-zero -- or, in the C++ phase, leaves an empty or truncated output (a
# prefix of the golden) -- is run again; a wrong but complete output fails at
# once, never retried. Every retried test is listed at the end, so a flake
# stays visible without failing the board. A long retried list means: free
# memory on the machine before trusting the run's timings, and re-run a
# failing heavy test on its own.
retried_names=()
vvp_once_more() {   # vvp_once_more <name> <file.vvp>: re-run in the cwd (twice at most), 0 if it exits 0
    retried_names+=("$1")
    "$VVP" "$2" >/dev/null 2>&1 && return 0
    "$VVP" "$2" >/dev/null 2>&1
}
# The same rule for a pass that reads what vvp prints (tb_fifo, tb_dcfifo,
# KEYPS2, VGATEXT): run it in <dir> and leave its output in VVP_OUT, run again
# (twice at most) only when vvp exits non-zero. Not called in $(...), so that
# retried_names keeps the name.
vvp_capture() {    # vvp_capture <name> <dir> <file.vvp>: VVP_OUT, and vvp's exit status
    local rc n=0
    VVP_OUT=$(cd "$2" && "$VVP" -n "$3" 2>&1); rc=$?
    while [ $rc -ne 0 ] && [ $n -lt 2 ]; do
        [ $n -eq 0 ] && retried_names+=("$1")
        VVP_OUT=$(cd "$2" && "$VVP" -n "$3" 2>&1); rc=$?
        n=$((n + 1))
    done
    return $rc
}

# Examples to exclude from the standalone simulation phase (still run cmmcomp
# .asm compare AND asmcomp so their .v + .mif are available for any
# downstream project-level link):
#   procBlind/procBlindOpt - take too long to simulate (heavy FISTA)
#   ArcTan/Seno/Sqrt   - Math benchmarks: compute internally with no out(...) calls
#   ProcDTW/ZeroCross  - real testbench lives in the multi-proc DTW project pass
#   cmm_define         - synthetic #define fixture; while(1) testbench loop,
#                        asm-golden compare is enough to lock the lexer expansion
#   ResetCheck         - real testbench is the directed reset pass (pulses rst
#                        mid-run); the generated standalone tb would idle in while(1)
#   link_*             - toma/cade need a partner: alone they wait forever; the
#                        real testbench is the LINK project pass
#   sapho_all_a/_b     - the same, real testbench is the LINKALL project pass
#   lk_*               - the same, real testbench is the LINK3 project pass
SIM_SKIP=("procBlind" "procBlindOpt" "ArcTan" "Seno" "Sqrt" "ProcDTW" "ZeroCross" "cmm_define" "ResetCheck"
          "link_envia" "link_recebe" "link_ping" "link_pong" "sapho_all_a" "sapho_all_b"
          "lk_chain_a" "lk_chain_b" "lk_chain_c" "lk_fan_w" "lk_echo" "lk_count" "lk_count7" "lk_merge"
          "lk_flt_w" "lk_flt_r")

# Examples to also skip the asmcomp build for (their .v / .mif are
# never consumed elsewhere - no standalone sim, no project link).
BUILD_SKIP=("procBlind" "procBlindOpt" "ArcTan" "Seno" "Sqrt" "cmm_define")
build_skipped() {
    local name="$1"
    for s in "${BUILD_SKIP[@]}"; do [ "$s" = "$name" ] && return 0; done
    return 1
}
sim_skipped() {
    local name="$1"
    for s in "${SIM_SKIP[@]}"; do [ "$s" = "$name" ] && return 0; done
    return 1
}

# hardware.txt (optional, next to a test's golden): one "<PARAM> <value>" per
# line, held to the parameters the generated top passes the processor (an
# opcode parameter left out of the top counts as 0); "<N" asks for less than
# N. It says what hardware the program must (not) get, e.g. "F_DIV 0" for a
# float division nobody calls, "MDATAS <64" for a heap that must not be there.
# Prints the first mismatch and returns 1, or returns 0.
hw_check() {   # hw_check <hardware.txt> <top.v>
    [ -f "$1" ] || return 0
    local p want got
    while IFS=' ' read -r p want || [ -n "$p" ]; do   # the caller may have IFS=newline
        want="${want%$'\r'}"; [ -z "$p" ] && continue
        got=$(grep -oE "\.$p *\( *[0-9]+ *\)" "$2" | head -1 | sed -E 's/.*\( *([0-9]+) *\)/\1/')
        if [ "${want#<}" != "$want" ]; then                 # "<N": below N
            [ "${got:-0}" -lt "${want#<}" ] || { echo "hardware: $p is ${got:-0}, expected $want"; return 1; }
        else
            [ "${got:-0}" = "$want" ] || { echo "hardware: $p is ${got:-0}, expected $want"; return 1; }
        fi
    done < "$1"
    return 0
}

# A stack that reaches its depth in an Icarus run: the simulation's fl_full
# (core.v, stack; it catches a wrapping pointer too) went to 1 in the VCD.
# With the depths asmcomp works out (peak + 1) it never should; a declared
# depth that is too small shows up here even when the outputs happen to match.
# Returns 1 when a flag fired, 0 otherwise (and when there is no VCD).
stack_check() {   # stack_check <tb.vcd>
    [ -f "$1" ] || return 0
    awk '{ sub(/\r$/, "") }
         !hdr && $1 == "$var" && $5 == "fl_full" { id[$4] = 1; next }
         $1 == "$enddefinitions"                 { hdr = 1; next }
         hdr && substr($0, 1, 1) == "1" && (substr($0, 2) in id) { fired = 1; exit }
         END { exit fired ? 1 : 0 }' "$1"
}

# ---- 1. build the 4 binaries ------------------------------------------------

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "==> building cmmcomp + cppcomp + asmcomp + yanc"
    mkdir -p "$BIN_DIR"

    # Wipe stale binaries first: a silently-failing gcc must NOT leave the
    # previous build in place to fool the regress into testing dead code.
    rm -f "$CMMCOMP" "$CPPC" "$ASMCOMP" "$YANC"

    # Per-binary `set -e` so a build error aborts the script. Without it,
    # a gcc -Werror failure would print to the terminal but the regress
    # would happily continue and run all examples against the (now-missing)
    # binary, hiding the real problem.
    set -e

    # Windows: each tool carries the manifest that makes it work in UTF-8
    # (Compilers/common/utf8.rc, as the Makefile builds it)
    UTF8RES=""
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*)
            UTF8RES="$SCRATCH/utf8_res.o"
            (cd "$ROOT/Compilers/common" && windres -O coff utf8.rc "$UTF8RES") ;;
    esac

    pushd "$ROOT/Compilers/CMMComp/Sources" >/dev/null
    bison -y -d CMMComp.y
    flex CMMComp.l
    gcc -O2 -Wall -Werror -o "$CMMCOMP" \
        ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c \
        funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c \
        variaveis.c array_index.c global.c macros.c messages.c args.c dirac.c \
        y.tab.c ../../common/yanc_num.c ../../common/asm_share.c ../../common/proc_name.c ../../common/console_utf8.c ../../common/exe_dir.c ../../common/yanc_lang.c $UTF8RES -lm
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    pushd "$CPP_ROOT/Sources" >/dev/null
    bison -y -d CPPComp.y
    flex CPPComp.l
    gcc -O2 -Wall \
        $CPP_DEFS -o "$CPPC" \
        main.c messages.c types.c symtab.c ast.c codegen.c cpppp.c lex.yy.c y.tab.c \
        ../../common/asm_share.c ../../common/proc_name.c ../../common/console_utf8.c ../../common/exe_dir.c ../../common/yanc_lang.c $UTF8RES -lm
    rm -f lex.yy.c y.tab.c y.tab.h
    popd >/dev/null

    pushd "$ROOT/Compilers/ASMComp/Sources" >/dev/null
    flex -o ASMComp.c ASMComp.l
    flex -o pass1_lex.c pass1.l
    gcc -O2 -Wall -Werror -o "$ASMCOMP" \
        ASMComp.c pass1_lex.c pass1.c eval.c labels.c opcodes.c variaveis.c t2t.c \
        hdl.c simulacao.c array.c messages.c args.c ../../common/yanc_num.c ../../common/asm_share.c ../../common/proc_name.c ../../common/console_utf8.c ../../common/exe_dir.c ../../common/yanc_lang.c $UTF8RES -lm
    rm -f ASMComp.c pass1_lex.c
    popd >/dev/null

    rm -f "$YANC"
    gcc -O2 -Wall -Werror -o "$YANC" "$ROOT/Compilers/Driver/yanc.c" \
        "$ROOT/Compilers/common/console_utf8.c" "$ROOT/Compilers/common/exe_dir.c" "$ROOT/Compilers/common/yanc_lang.c" $UTF8RES

    set +e
fi

mkdir -p "$WORK_DIR" "$WORK_CPP" "$TMP_DIR"

# Accumulators for the combined summary at the end.
pass=0
fail=0
failed_names=()

# ---- 2. CMM phase ----------------------------------------------------------

if [ "$CPP_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CMM phase"

    # Each test is a folder Compilers/CMMComp/Tests/<prname>/ holding
    # <prname>.cmm, its own files (array inits, input_N.txt) and golden.asm +
    # golden_sim/. Project-pass wrappers (DTW, LINK, FIFO, ...) have no
    # <prname>.cmm and are left out. The test is copied to $WORK_DIR/<prname>
    # and compiled into $WORK_DIR/<prname>/<prname>/, its output folder.
    shopt -s globstar nullglob
    cmm_files=()
    for d in "$CMM_ROOT"/Tests/*/; do
        d="${d%/}"; [ -f "$d/$(basename "$d").cmm" ] && cmm_files+=("$d/$(basename "$d").cmm")
    done
    IFS=$'\n' cmm_sorted=($(printf '%s\n' "${cmm_files[@]}" | sort))

    for cmm in "${cmm_sorted[@]}"; do
        proc_dir_rel="$(dirname "$cmm")"
        prname="$(basename "$proc_dir_rel")"
        filename="$(basename "$cmm")"

        work_proc="$WORK_DIR/$prname"
        rm -rf "$work_proc"
        cp -r "$proc_dir_rel" "$work_proc"

        out="$work_proc/$prname"            # the output folder
        tmp="$out"

        asm_file="$out/$prname.asm"
        golden="$proc_dir_rel/golden.asm"

        # cmmcomp step ------------------------------------------------------
        if ! "$CMMCOMP" -o "$out" "$work_proc/$filename" >/dev/null 2>&1; then
            echo "FAIL ($prname): cmmcomp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi
        [ -s "$asm_file" ] || { echo "FAIL ($prname): asm missing/empty"; fail=$((fail+1)); failed_names+=("$prname"); continue; }

        # size ratchet ------------------------------------------------------
        num_ins=$(grep "^num_ins " "$tmp/prog_log.txt" 2>/dev/null | awk '{print $2}')
        SIZE_CURRENT[$prname]="${num_ins:-0}"
        baseline="${SIZE_BASELINE[$prname]:-}"
        size_grew=0
        if [ -z "$baseline" ]; then
            size_msg="  [size: ${num_ins:-?}, no baseline]"
            # a new example enters with its count, or the ratchet never holds it
            if [ "$UPDATE_SIZE" -eq 0 ]; then
                echo "FAIL ($prname): no size baseline; add \"$prname ${num_ins:-?}\" to $SIZE_BASELINE_FILE"
                fail=$((fail + 1)); failed_names+=("$prname"); continue
            fi
        elif [ "${num_ins:-0}" -gt "$baseline" ]; then
            size_msg="  [size: $num_ins, GREW +$((num_ins - baseline)) vs $baseline]"
            size_grew=1
        elif [ "${num_ins:-0}" -lt "$baseline" ]; then
            size_msg="  [size: $num_ins, -$((baseline - num_ins)) vs $baseline]"
        else
            size_msg="  [size: $num_ins]"
        fi
        if [ "$size_grew" -eq 1 ] && [ "$UPDATE_SIZE" -eq 0 ]; then
            echo "FAIL ($prname):$size_msg"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        # .asm capture or compare -------------------------------------------
        if [ "$UPDATE" -eq 1 ]; then
            cp "$asm_file" "$golden"
            asm_status="UPDATED"
        elif [ ! -f "$golden" ]; then
            echo "FAIL ($prname): no golden at $golden (run --update?)"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        elif cmp -s "$asm_file" "$golden"; then
            asm_status="PASS"
        else
            echo "FAIL ($prname): asm differs from golden"
            diff "$golden" "$asm_file" | head -20 | sed 's/^/    /'
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        # simulation phase --------------------------------------------------
        if [ "$NO_SIM" -eq 1 ] || build_skipped "$prname"; then
            echo "$asm_status ($prname)  [sim skipped]$size_msg"
            pass=$((pass + 1))
            continue
        fi

        if ! "$ASMCOMP" -f 100 -c 100000 "$asm_file" >/dev/null 2>&1; then
            echo "FAIL ($prname): asmcomp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi
        if ! hw_msg=$(hw_check "$proc_dir_rel/hardware.txt" "$out/$prname.v"); then
            echo "FAIL ($prname): $hw_msg"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        # Procs whose real testbench lives at the project level (DTW): build their
        # .v/.mif here for the project pass to link, skip standalone iverilog.
        if sim_skipped "$prname"; then
            echo "$asm_status ($prname)  [standalone sim skipped]$size_msg"
            pass=$((pass + 1))
            continue
        fi

        uproc="$out/$prname"
        tb="$tmp/${prname}_tb.v"
        if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
            echo "FAIL ($prname): asmcomp artifacts missing"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        if ! "$IVERILOG" -s "${prname}_tb" -o "$tmp/$prname.vvp" \
                "$tb" "$uproc.v" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
            echo "FAIL ($prname): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        pushd "$out" >/dev/null
        "$VVP" "$tmp/$prname.vvp" >/dev/null 2>&1
        vvp_status=$?
        if [ $vvp_status -ne 0 ]; then vvp_once_more "$prname" "$tmp/$prname.vvp"; vvp_status=$?; fi
        popd >/dev/null
        if [ $vvp_status -ne 0 ]; then
            echo "FAIL ($prname): vvp exited non-zero"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi
        if ! stack_check "$tmp/${prname}_tb.vcd"; then
            echo "FAIL ($prname): a stack reached its depth in the simulation (fl_full)"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        sim_dir="$out"
        golden_sim="$proc_dir_rel/golden_sim"
        out_files=("$sim_dir"/output_*.txt)
        if [ "${#out_files[@]}" -eq 0 ] || [ ! -e "${out_files[0]}" ]; then
            echo "FAIL ($prname): testbench produced no output_*.txt"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        if [ "$UPDATE" -eq 1 ]; then
            rm -rf "$golden_sim"
            mkdir -p "$golden_sim"
            for f in "${out_files[@]}"; do cp "$f" "$golden_sim/"; done
            echo "$asm_status ($prname)  [sim UPDATED, ${#out_files[@]} file(s)]$size_msg"
            pass=$((pass + 1))
            continue
        fi

        if [ ! -d "$golden_sim" ]; then
            echo "FAIL ($prname): no sim golden at $golden_sim (run --update?)"
            fail=$((fail + 1)); failed_names+=("$prname"); continue
        fi

        sim_fail=0
        for f in "${out_files[@]}"; do
            base="$(basename "$f")"
            if ! cmp -s "$f" "$golden_sim/$base"; then
                echo "FAIL ($prname): $base differs from sim golden"
                sim_fail=1
            fi
        done
        if [ $sim_fail -eq 0 ]; then
            echo "$asm_status ($prname)  [sim OK]$size_msg"
            pass=$((pass + 1))
        else
            fail=$((fail + 1)); failed_names+=("$prname")
        fi
    done

    # ---- 3. project pass: multi-proc DTW with top-level testbench ----------

    if [ "$NO_SIM" -eq 0 ]; then
        proj="DTW"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        golden_proj="$CMM_ROOT/Tests/$proj/golden_sim"
        procs=("ProcDTW" "ZeroCross")

        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"

        project_ok=1
        proc_vs=()
        for p in "${procs[@]}"; do
            if [ ! -s "$WORK_DIR/$p/$p/$p.v" ]; then
                echo "FAIL ($proj): $p artifacts missing - did its build pass?"
                project_ok=0; break
            fi
            proc_vs+=("$WORK_DIR/$p/$p/$p.v")
            cp "$WORK_DIR/$p/$p/$p.v"          "$proj_tmp/"
            cp "$WORK_DIR/$p/$p/${p}_data.mif" "$proj_tmp/"
            cp "$WORK_DIR/$p/$p/${p}_inst.mif" "$proj_tmp/"
        done

        if [ "$project_ok" -eq 1 ]; then
            cp "$proj_top"/*.txt "$proj_tmp/" 2>/dev/null || true

            if ! "$IVERILOG" -s top_level_tb -o "$proj_tmp/$proj.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" "$HDL/myFIFO.v" \
                    "${proc_vs[@]}" \
                    "$proj_top"/*.v >/dev/null 2>&1; then
                echo "FAIL ($proj): iverilog exited non-zero"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            fi
        fi

        if [ "$project_ok" -eq 1 ]; then
            pushd "$proj_tmp" >/dev/null
            # No waveform here: the tb gates $dumpvars behind +WAVE, so we run
            # without it. Dumping every signal of this heavy multi-proc sim
            # through the FST writer intermittently crashed vvp on Windows
            # (exit 1, empty stderr) -- a flaky DTW failure -- and the dump is
            # pure overhead since this pass only compares the output_*.txt logs,
            # never the trace. multi_proc (--sim iverilog|verilator) passes +WAVE
            # for the interactive GTKWave flow.
            "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
            vvp_status=$?
            if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/$proj.vvp"; vvp_status=$?; fi
            popd >/dev/null
            if [ $vvp_status -ne 0 ]; then
                echo "FAIL ($proj): vvp exited non-zero"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            fi
        fi

        if [ "$project_ok" -eq 1 ]; then
            out_files=("$proj_tmp"/output_*.txt)
            if [ "${#out_files[@]}" -eq 0 ] || [ ! -e "${out_files[0]}" ]; then
                echo "FAIL ($proj): testbench produced no output_*.txt"
                fail=$((fail + 1)); failed_names+=("$proj")
            elif [ "$UPDATE" -eq 1 ]; then
                rm -rf "$golden_proj"; mkdir -p "$golden_proj"
                for f in "${out_files[@]}"; do cp "$f" "$golden_proj/"; done
                echo "UPDATED ($proj)  [sim UPDATED, ${#out_files[@]} file(s)]"
                pass=$((pass + 1))
            elif [ ! -d "$golden_proj" ]; then
                echo "FAIL ($proj): no sim golden at $golden_proj (run --update?)"
                fail=$((fail + 1)); failed_names+=("$proj")
            else
                sim_fail=0
                for f in "${out_files[@]}"; do
                    base="$(basename "$f")"
                    if ! cmp -s "$f" "$golden_proj/$base"; then
                        echo "FAIL ($proj): $base differs from sim golden"
                        sim_fail=1
                    fi
                done

                # Correctness anchor: a working DTW emits 459908 on its
                # first DTW output port at @out_en. The byte-equal golden
                # compare above catches regressions only if someone hasn't
                # rerun --update; this assertion stays load-bearing even
                # then, and gives a useful failure message tied to the
                # algorithm rather than to a blob diff.
                anchor_file="$proj_tmp/output_dtw_1.txt"
                anchor_expect="459908"
                anchor_got=$(tr -d '[:space:]' < "$anchor_file" 2>/dev/null)
                if [ "$anchor_got" != "$anchor_expect" ]; then
                    echo "FAIL ($proj): output_dtw_1 expected '$anchor_expect' got '$anchor_got'"
                    sim_fail=1
                fi

                if [ $sim_fail -eq 0 ]; then
                    echo "PASS ($proj)  [sim OK, output_dtw_1=$anchor_expect]"
                    pass=$((pass + 1))
                else
                    fail=$((fail + 1)); failed_names+=("$proj")
                fi
            fi
        fi
    fi

    # ---- 3a. project pass: two SAPHOs over toma/cade ------------------------
    # docs/design/toma-and-cade.md. link_envia hands 16 values to link_recebe (their
    # waits cross over, so each side arrives first in turn); link_ping and
    # link_pong use one link both ways. Besides the sim golden, the outputs are
    # checked against the sequences computed here (n*7+3, n+1), which stay
    # load-bearing after an --update.

    if [ "$NO_SIM" -eq 0 ]; then
        proj="LINK"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        golden_proj="$CMM_ROOT/Tests/$proj/golden_sim"
        procs=("link_envia" "link_recebe" "link_ping" "link_pong")

        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"

        project_ok=1
        proc_vs=()
        for p in "${procs[@]}"; do
            if [ ! -s "$WORK_DIR/$p/$p/$p.v" ]; then
                echo "FAIL ($proj): $p artifacts missing - did its build pass?"
                project_ok=0; break
            fi
            proc_vs+=("$WORK_DIR/$p/$p/$p.v")
            cp "$WORK_DIR/$p/$p/${p}_data.mif" "$proj_tmp/"
            cp "$WORK_DIR/$p/$p/${p}_inst.mif" "$proj_tmp/"
        done

        # the C++ partners of the mixed pairs: built here, as the CPP phase
        # builds its tests (it runs after this pass)
        for p in link_cpp_recebe link_cpp_ping; do
            [ "$project_ok" -eq 1 ] || break
            cp_dir="$proj_tmp/cpp/$p"
            if ! "$CPPC" -o "$cp_dir" "$proj_top/../cpp/$p.cpp" >/dev/null 2>&1 ||
               ! "$ASMCOMP" -f 100 -c 100000 "$cp_dir/$p.asm" >/dev/null 2>&1; then
                echo "FAIL ($proj): building the C++ partner $p"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0; break
            fi
            proc_vs+=("$cp_dir/$p.v")
            cp "$cp_dir/${p}_data.mif" "$cp_dir/${p}_inst.mif" "$proj_tmp/"
        done

        if [ "$project_ok" -eq 1 ] && ! "$IVERILOG" -s link_tb -o "$proj_tmp/$proj.vvp" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" "${proc_vs[@]}" \
                "$proj_top/link_tb.v" >/dev/null 2>&1; then
            echo "FAIL ($proj): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
        fi

        if [ "$project_ok" -eq 1 ]; then
            pushd "$proj_tmp" >/dev/null
            "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
            vvp_status=$?
            if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/$proj.vvp"; vvp_status=$?; fi
            popd >/dev/null
            if [ $vvp_status -ne 0 ]; then
                echo "FAIL ($proj): vvp exited non-zero"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            fi
        fi

        if [ "$project_ok" -eq 1 ]; then
            if [ "$UPDATE" -eq 1 ]; then
                rm -rf "$golden_proj"; mkdir -p "$golden_proj"
                cp "$proj_tmp"/output_*.txt "$golden_proj/"
                echo "UPDATED ($proj)  [sim UPDATED]"
            fi
            sim_fail=0
            for f in output_link.txt output_pingpong.txt output_link_cpp.txt output_pingpong_cpp.txt; do
                if [ ! -s "$proj_tmp/$f" ]; then
                    echo "FAIL ($proj): the testbench wrote no $f"; sim_fail=1
                elif ! cmp -s "$proj_tmp/$f" "$golden_proj/$f"; then
                    echo "FAIL ($proj): $f differs from sim golden"; sim_fail=1
                fi
            done
            # the sequences themselves: every value once, in order
            want_link=$(for n in $(seq 0 15); do echo $((n * 7 + 3)); done | tr '\n' ' ')
            want_pp=$(seq 1 16 | tr '\n' ' ')
            got_link=$(tr -d '\r' < "$proj_tmp/output_link.txt" 2>/dev/null | tr '\n' ' ')
            got_pp=$(tr -d '\r' < "$proj_tmp/output_pingpong.txt" 2>/dev/null | tr '\n' ' ')
            if [ "$got_link" != "$want_link" ]; then
                echo "FAIL ($proj): envia->recebe got '$got_link', want '$want_link'"; sim_fail=1
            fi
            if [ "$got_pp" != "$want_pp" ]; then
                echo "FAIL ($proj): ping<->pong got '$got_pp', want '$want_pp'"; sim_fail=1
            fi
            got_link_cpp=$(tr -d '\r' < "$proj_tmp/output_link_cpp.txt" 2>/dev/null | tr '\n' ' ')
            got_pp_cpp=$(tr -d '\r' < "$proj_tmp/output_pingpong_cpp.txt" 2>/dev/null | tr '\n' ' ')
            if [ "$got_link_cpp" != "$want_link" ]; then
                echo "FAIL ($proj): envia(C+-)->recebe(C++) got '$got_link_cpp', want '$want_link'"; sim_fail=1
            fi
            if [ "$got_pp_cpp" != "$want_pp" ]; then
                echo "FAIL ($proj): ping(C++)<->pong(C+-) got '$got_pp_cpp', want '$want_pp'"; sim_fail=1
            fi
            if [ $sim_fail -eq 0 ]; then
                echo "PASS ($proj)  [sim OK, 16 values each way, in order; C+- and C+-/C++ pairs]"
                pass=$((pass + 1))
            else
                fail=$((fail + 1)); failed_names+=("$proj")
            fi
        fi

        # the same programs on two clocks (20 ns and 27.3 ns), LINKAS = 1
        # (docs/design/toma-and-cade.md section 8): checked against the sequences only
        if [ "$project_ok" -eq 1 ]; then
            name2="LINK2"
            if ! "$IVERILOG" -s link2_tb -o "$proj_tmp/$name2.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" "${proc_vs[@]}" \
                    "$proj_top/link2_tb.v" >/dev/null 2>&1; then
                echo "FAIL ($name2): iverilog exited non-zero"
                fail=$((fail + 1)); failed_names+=("$name2")
            else
                pushd "$proj_tmp" >/dev/null
                "$VVP" "$proj_tmp/$name2.vvp" >/dev/null 2>&1
                vvp_status=$?
                if [ $vvp_status -ne 0 ]; then vvp_once_more "$name2" "$proj_tmp/$name2.vvp"; vvp_status=$?; fi
                popd >/dev/null
                sim_fail=0
                if [ $vvp_status -ne 0 ]; then
                    echo "FAIL ($name2): vvp exited non-zero"; sim_fail=1
                else
                    want_link=$(for n in $(seq 0 15); do echo $((n * 7 + 3)); done | tr '\n' ' ')
                    want_pp=$(seq 1 16 | tr '\n' ' ')
                    for f in output_link2_fs.txt output_link2_sf.txt output_pingpong2.txt; do
                        want="$want_link"; [ "$f" = output_pingpong2.txt ] && want="$want_pp"
                        got=$(tr -d '\r' < "$proj_tmp/$f" 2>/dev/null | tr '\n' ' ')
                        if [ "$got" != "$want" ]; then
                            echo "FAIL ($name2): $f got '$got', want '$want'"; sim_fail=1
                        fi
                    done
                fi
                if [ $sim_fail -eq 0 ]; then
                    echo "PASS ($name2)  [two clocks, 20 ns / 27.3 ns: 16 values each way, in order]"
                    pass=$((pass + 1))
                else
                    fail=$((fail + 1)); failed_names+=("$name2")
                fi
            fi
        fi
    fi

    # ---- 3a. project pass LINKALL: the whole processor over toma/cade -------
    # sapho_all_a hands its result to sapho_all_b, on one clock (LINKAS 0) and
    # on two (LINKAS 1); sapho_all alone runs with its output fed back
    # (input = last output XOR 7). Checked against model.py only: A's lines are
    # model(7), B's model(model(7)), and each fed-back line is model of the
    # previous one XOR 7. The two programs must stay sapho_all plus the lines
    # marked // LINK (checked first, also under --no-sim).
    sa_dir="$CMM_ROOT/Tests/sapho_all"
    mkdir -p "$TMP_DIR"
    grep -v -E '// LINK|^#PRNAME' "$sa_dir/sapho_all.cmm" > "$TMP_DIR/linkall_src0.txt"
    for p in sapho_all_a sapho_all_b; do
        # temp files, not <(...): MSYS2's diff/cmp can't always open /proc paths
        grep -v -E '// LINK|^#PRNAME' "$CMM_ROOT/Tests/$p/$p.cmm" > "$TMP_DIR/linkall_src1.txt"
        if ! diff "$TMP_DIR/linkall_src0.txt" "$TMP_DIR/linkall_src1.txt" >/dev/null; then
            echo "FAIL (LINKALL-source): $p.cmm differs from sapho_all.cmm beyond its // LINK lines"
            fail=$((fail + 1)); failed_names+=("LINKALL-source")
        else
            pass=$((pass + 1))
        fi
    done

    if [ "$NO_SIM" -eq 0 ]; then
        proj="LINKALL"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"
        project_ok=1
        proc_vs=()
        for p in sapho_all sapho_all_a sapho_all_b; do
            if [ ! -s "$WORK_DIR/$p/$p/$p.v" ]; then
                echo "FAIL ($proj): $p artifacts missing - did its build pass?"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0; break
            fi
            proc_vs+=("$WORK_DIR/$p/$p/$p.v")
            cp "$WORK_DIR/$p/$p/${p}_data.mif" "$WORK_DIR/$p/$p/${p}_inst.mif" "$proj_tmp/"
        done
        if [ "$project_ok" -eq 1 ] && ! "$IVERILOG" -s linkall_tb -o "$proj_tmp/$proj.vvp" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" "${proc_vs[@]}" \
                "$proj_top/linkall_tb.v" >/dev/null 2>&1; then
            echo "FAIL ($proj): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
        fi
        if [ "$project_ok" -eq 1 ]; then
            pushd "$proj_tmp" >/dev/null
            "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
            vvp_status=$?
            if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/$proj.vvp"; vvp_status=$?; fi
            popd >/dev/null
            if [ $vvp_status -ne 0 ]; then
                echo "FAIL ($proj): vvp exited non-zero"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            fi
        fi
        if [ "$project_ok" -eq 1 ]; then
            sim_fail=0
            la_a=$(python3 "$sa_dir/model.py" 7)
            la_b=$(python3 "$sa_dir/model.py" "$la_a")
            for f in a1 b1 a2 b2; do
                want=$la_a; [ "${f:0:1}" = b ] && want=$la_b
                lines=$(grep -c '' "$proj_tmp/output_$f.txt" 2>/dev/null)
                other=$(tr -d '\r' < "$proj_tmp/output_$f.txt" 2>/dev/null | grep -cvx -- "$want")
                if [ "${lines:-0}" -lt 12 ] || [ "$other" -ne 0 ]; then
                    echo "FAIL ($proj): output_$f.txt has ${lines:-0} lines, $other not $want"; sim_fail=1
                fi
            done
            prev=0; n=0
            while IFS= read -r v; do
                v="${v%$'\r'}"
                want=$(python3 "$sa_dir/model.py" $((prev ^ 7)))
                if [ "$v" != "$want" ]; then
                    echo "FAIL ($proj): fed back, line $((n + 1)) is $v, want model($((prev ^ 7))) = $want"; sim_fail=1; break
                fi
                prev=$v; n=$((n + 1))
            done < "$proj_tmp/output_fb.txt"
            [ "$n" -ge 12 ] || { [ "$sim_fail" -eq 1 ] || echo "FAIL ($proj): fed back, only $n lines"; sim_fail=1; }
            if [ $sim_fail -eq 0 ]; then
                echo "PASS ($proj)  [sapho_all A -> B: one clock and two clocks, A = $la_a, B = $la_b; alone fed back, $n turns = model.py]"
                pass=$((pass + 1))
            else
                fail=$((fail + 1)); failed_names+=("$proj")
            fi
        fi
    fi

    # ---- 3a. project pass LINK3: several links per processor ----------------
    # docs/design/toma-and-cade-links.md, on 20 ns / 27.3 ns clocks: a chain A -> B -> C
    # (A->B with the _sync forms, B->C synchronized; once more with the C++
    # middle lk_cpp_chain_b), fan-out (two TOM links), fan-in (two CAD links)
    # and a float word over a link. Checked against the sequences computed here.
    if [ "$NO_SIM" -eq 0 ]; then
        proj="LINK3"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"
        project_ok=1
        proc_vs=()
        for p in lk_chain_a lk_chain_b lk_chain_c lk_fan_w lk_echo lk_count lk_count7 lk_merge lk_flt_w lk_flt_r; do
            if [ ! -s "$WORK_DIR/$p/$p/$p.v" ]; then
                echo "FAIL ($proj): $p artifacts missing - did its build pass?"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0; break
            fi
            proc_vs+=("$WORK_DIR/$p/$p/$p.v")
            cp "$WORK_DIR/$p/$p/${p}_data.mif" "$WORK_DIR/$p/$p/${p}_inst.mif" "$proj_tmp/"
        done
        p=lk_cpp_chain_b
        if [ "$project_ok" -eq 1 ]; then
            cp_dir="$proj_tmp/cpp/$p"
            if ! "$CPPC" -o "$cp_dir" "$proj_top/../cpp/$p.cpp" >/dev/null 2>&1 ||
               ! "$ASMCOMP" -f 100 -c 100000 "$cp_dir/$p.asm" >/dev/null 2>&1; then
                echo "FAIL ($proj): building the C++ middle $p"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            else
                proc_vs+=("$cp_dir/$p.v")
                cp "$cp_dir/${p}_data.mif" "$cp_dir/${p}_inst.mif" "$proj_tmp/"
            fi
        fi
        if [ "$project_ok" -eq 1 ] && ! "$IVERILOG" -s link3_tb -o "$proj_tmp/$proj.vvp" \
                "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                "$HDL/core.v" "$HDL/ula.v" "${proc_vs[@]}" \
                "$proj_top/link3_tb.v" >/dev/null 2>&1; then
            echo "FAIL ($proj): iverilog exited non-zero"
            fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
        fi
        if [ "$project_ok" -eq 1 ]; then
            pushd "$proj_tmp" >/dev/null
            "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
            vvp_status=$?
            if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/$proj.vvp"; vvp_status=$?; fi
            popd >/dev/null
            if [ $vvp_status -ne 0 ]; then
                echo "FAIL ($proj): vvp exited non-zero"
                fail=$((fail + 1)); failed_names+=("$proj"); project_ok=0
            fi
        fi
        if [ "$project_ok" -eq 1 ]; then
            sim_fail=0
            for f in chain chain2 fan0 fan1 merge float; do
                case $f in
                    chain|chain2) want=$(for n in $(seq 0 15); do echo $(((n * 3 + 1) * 2)); done | tr '\n' ' ') ;;
                    fan0)         want=$(seq 0 15 | tr '\n' ' ') ;;
                    fan1)         want=$(seq 100 115 | tr '\n' ' ') ;;
                    merge)        want=$(for n in $(seq 0 15); do echo $((n * 1000 + n * 7)); done | tr '\n' ' ') ;;
                    float)        want=$(for n in $(seq 0 15); do echo $((n * 3)); done | tr '\n' ' ') ;;
                esac
                got=$(tr -d '\r' < "$proj_tmp/output_$f.txt" 2>/dev/null | tr '\n' ' ')
                if [ "$got" != "$want" ]; then
                    echo "FAIL ($proj): output_$f.txt got '$got', want '$want'"; sim_fail=1
                fi
            done
            if [ $sim_fail -eq 0 ]; then
                echo "PASS ($proj)  [chains (C+- and C++ middle), fan-out, fan-in, float: 16 values each, in order]"
                pass=$((pass + 1))
            else
                fail=$((fail + 1)); failed_names+=("$proj")
            fi
        fi
    fi

    # ---- 3a. project pass FIFO: a SAPHO writes into SAPHO/myFIFO.v ---------
    # fifo_w's out() feeds the FIFO (wrreq = out_en, data = out), back-to-back
    # OUTs and OUTs from loops; the tb drains it. Plus Scripts/hw/tb_fifo.v,
    # the FIFO alone (full refuses, read right after a write). Then the same
    # across two clocks: myDCFIFO between fifo_w and a SAPHO reader
    # (Tests/FIFO/fifo_r.cmm), and Scripts/hw/tb_dcfifo.v. docs/design/myfifo.md.

    if [ "$NO_SIM" -eq 0 ]; then
        proj="FIFO"
        proj_top="$CMM_ROOT/Tests/$proj/TopLevel"
        proj_tmp="$TMP_DIR/$proj"
        rm -rf "$proj_tmp"; mkdir -p "$proj_tmp"
        p="fifo_w"
        if [ ! -s "$WORK_DIR/$p/$p/$p.v" ]; then
            echo "FAIL ($proj): $p artifacts missing - did its build pass?"
            fail=$((fail + 1)); failed_names+=("$proj")
        else
            cp "$WORK_DIR/$p/$p/${p}_data.mif" "$WORK_DIR/$p/$p/${p}_inst.mif" "$proj_tmp/"
            sim_fail=0
            if ! "$IVERILOG" -s fifo_tb -o "$proj_tmp/$proj.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" "$HDL/myFIFO.v" \
                    "$WORK_DIR/$p/$p/$p.v" "$proj_top/fifo_tb.v" >/dev/null 2>&1; then
                echo "FAIL ($proj): iverilog exited non-zero"; sim_fail=1
            else
                pushd "$proj_tmp" >/dev/null
                "$VVP" "$proj_tmp/$proj.vvp" >/dev/null 2>&1
                vvp_status=$?
                if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/$proj.vvp"; vvp_status=$?; fi
                popd >/dev/null
                want="1 2 3 4 5 6 7 8 11 12 13 14 "
                got=$(tr -d '\r' < "$proj_tmp/output_fifo.txt" 2>/dev/null | tr '\n' ' ')
                if [ $vvp_status -ne 0 ]; then
                    echo "FAIL ($proj): vvp exited non-zero"; sim_fail=1
                elif [ "$got" != "$want" ]; then
                    echo "FAIL ($proj): the FIFO held '$got', want '$want'"; sim_fail=1
                fi
            fi
            # the FIFO alone (Scripts/hw/tb_fifo.v prints "0 of 3 failed")
            if [ $sim_fail -eq 0 ]; then
                if ! "$IVERILOG" -g2012 -s tb_fifo -o "$proj_tmp/tb_fifo.vvp" \
                        "$ROOT/Scripts/hw/tb_fifo.v" "$HDL/myFIFO.v" >/dev/null 2>&1; then
                    echo "FAIL ($proj): iverilog on tb_fifo.v exited non-zero"; sim_fail=1
                else
                    vvp_capture "$proj" "$proj_tmp" "$proj_tmp/tb_fifo.vvp"; tbf=$VVP_OUT
                    if ! printf '%s' "$tbf" | grep -q "^0 of 3 failed"; then
                        echo "FAIL ($proj): tb_fifo.v:"; printf '%s\n' "$tbf" | grep FAIL; sim_fail=1
                    fi
                fi
            fi
            # two clocks: myDCFIFO between a SAPHO writer and a SAPHO reader
            # (Tests/FIFO/fifo_r.cmm, built here), both clock ratios; then
            # Scripts/hw/tb_dcfifo.v both ways round ("0 of 4 failed")
            if [ $sim_fail -eq 0 ]; then
                r="fifo_r"; rd="$proj_tmp/$r"; mkdir -p "$rd"
                if ! "$CMMCOMP" -o "$rd" "$proj_top/../$r.cmm" >/dev/null 2>&1 ||
                   ! "$ASMCOMP" -f 100 -c 100000 "$rd/$r.asm" >/dev/null 2>&1; then
                    echo "FAIL ($proj): building the reader $r"; sim_fail=1
                else
                    cp "$rd/${r}_data.mif" "$rd/${r}_inst.mif" "$proj_tmp/"
                fi
            fi
            if [ $sim_fail -eq 0 ]; then
                if ! "$IVERILOG" -s dcfifo_tb -o "$proj_tmp/dcfifo.vvp" \
                        "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                        "$HDL/core.v" "$HDL/ula.v" "$HDL/myDCFIFO.v" \
                        "$WORK_DIR/$p/$p/$p.v" "$rd/$r.v" \
                        "$proj_top/dcfifo_tb.v" >/dev/null 2>&1; then
                    echo "FAIL ($proj): iverilog on dcfifo_tb.v exited non-zero"; sim_fail=1
                else
                    pushd "$proj_tmp" >/dev/null
                    "$VVP" "$proj_tmp/dcfifo.vvp" >/dev/null 2>&1
                    vvp_status=$?
                    if [ $vvp_status -ne 0 ]; then vvp_once_more "$proj" "$proj_tmp/dcfifo.vvp"; vvp_status=$?; fi
                    popd >/dev/null
                    if [ $vvp_status -ne 0 ]; then
                        echo "FAIL ($proj): vvp on dcfifo_tb.v exited non-zero"; sim_fail=1
                    fi
                    for f in output_dcfifo_fs.txt output_dcfifo_sf.txt; do
                        got=$(tr -d '\r' < "$proj_tmp/$f" 2>/dev/null | tr '\n' ' ')
                        if [ "$got" != "$want" ]; then
                            echo "FAIL ($proj): $f got '$got', want '$want'"; sim_fail=1
                        fi
                    done
                fi
            fi
            if [ $sim_fail -eq 0 ]; then
                for ratio in "-Ptb_dcfifo.WH=10 -Ptb_dcfifo.RH=13.65" "-Ptb_dcfifo.WH=13.65 -Ptb_dcfifo.RH=10"; do
                    # shellcheck disable=SC2086
                    if ! "$IVERILOG" -g2012 -s tb_dcfifo $ratio -o "$proj_tmp/tb_dcfifo.vvp" \
                            "$ROOT/Scripts/hw/tb_dcfifo.v" "$HDL/myDCFIFO.v" >/dev/null 2>&1; then
                        echo "FAIL ($proj): iverilog on tb_dcfifo.v exited non-zero"; sim_fail=1; break
                    fi
                    vvp_capture "$proj" "$proj_tmp" "$proj_tmp/tb_dcfifo.vvp"; tbf=$VVP_OUT
                    if ! printf '%s' "$tbf" | grep -q "^0 of 4 failed"; then
                        echo "FAIL ($proj): tb_dcfifo.v ($ratio):"; printf '%s\n' "$tbf" | grep FAIL; sim_fail=1
                    fi
                done
            fi
            if [ $sim_fail -eq 0 ]; then
                echo "PASS ($proj)  [one clock and two clocks: SAPHO -> FIFO -> 1..8, 11..14; tb_fifo 3/3, tb_dcfifo 4/4 both ways]"
                pass=$((pass + 1))
            else
                fail=$((fail + 1)); failed_names+=("$proj")
            fi
        fi
    fi

    # ---- 3a2. project pass KEYPS2: SAPHO/myKeyPS2.v on whole processors ------
    # One simulated keyboard drives two blocks: US on the C+- SAPHO kbd_cmm and
    # ABNT2 on the C++ SAPHO kbd_cpp, both with an 8-bit word (a character
    # needs no more), both echoing cade_sync(0) to port 0. The testbench checks
    # the characters itself (Tests/KEYPS2/TopLevel/keyps2_tb.v). The block
    # alone, every key and case: Scripts/hw/tb_keyps2.sh (step 4a0).
    # docs/design/ps2-keyboard.md, docs/peripherals.md.

    if [ "$NO_SIM" -eq 0 ]; then
        proj="KEYPS2"
        kd="$CMM_ROOT/Tests/$proj"
        kt="$TMP_DIR/$proj"
        rm -rf "$kt"; mkdir -p "$kt"
        k_fail=0
        if ! "$CMMCOMP" -o "$kt/kbd_cmm" "$kd/kbd_cmm.cmm" >/dev/null 2>&1 ||
           ! "$ASMCOMP" -f 100 -c 1000 "$kt/kbd_cmm/kbd_cmm.asm" >/dev/null 2>&1; then
            echo "FAIL ($proj): building kbd_cmm"; k_fail=1
        elif ! "$CPPC" -o "$kt/kbd_cpp" "$kd/kbd_cpp.cpp" >/dev/null 2>&1 ||
             ! "$ASMCOMP" -f 100 -c 1000 "$kt/kbd_cpp/kbd_cpp.asm" >/dev/null 2>&1; then
            echo "FAIL ($proj): building kbd_cpp"; k_fail=1
        else
            for p in kbd_cmm kbd_cpp; do
                cp "$kt/$p/${p}_data.mif" "$kt/$p/${p}_inst.mif" "$kt/$p/pc_${p}_mem.txt" "$kt/"
            done
            if ! "$IVERILOG" -s keyps2_tb -o "$kt/k.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" "$HDL/myKeyPS2.v" \
                    "$kt/kbd_cmm/kbd_cmm.v" "$kt/kbd_cpp/kbd_cpp.v" \
                    "$kd/TopLevel/keyps2_tb.v" >/dev/null 2>&1; then
                echo "FAIL ($proj): iverilog exited non-zero"; k_fail=1
            else
                vvp_capture "$proj" "$kt" "$kt/k.vvp"; k_out=$VVP_OUT
                if [ "$(printf '%s\n' "$k_out" | grep -c ': ok$')" -ne 2 ]; then
                    printf '%s\n' "$k_out" | grep "KEYPS2" | sed 's/^/    /'
                    echo "FAIL ($proj): a processor did not echo the keys"; k_fail=1
                fi
            fi
        fi
        if [ $k_fail -eq 0 ]; then
            echo "PASS ($proj)  [C+- (US) and C++ (ABNT2) SAPHOs, 8 bits, echo the keys through myKeyPS2]"
            pass=$((pass + 1))
        else
            fail=$((fail + 1)); failed_names+=("$proj")
        fi
    fi

    # ---- 3a3. project pass VGATEXT: myKeyPS2 and myVGAText on whole processors --
    # One simulated keyboard drives two systems, each a SAPHO of 8 bits with a
    # keyboard block (link 0, cade_sync) and a screen block (link 1,
    # toma_sync), all on 25 MHz: the C+- vga_cmm (US) and the C++ vga_cpp
    # (ABNT2), both echoing the keys to the screen; and a third, the 32-bit
    # C++ vga_io, which uses <iostream> on a keyboard and screen of its own.
    # The testbench reads each screen memory and checks the text and the cursor itself
    # (Tests/VGATEXT/TopLevel/vgatext_tb.v). The screen block alone:
    # Scripts/hw/tb_vgatext.sh (step 4a0). docs/design/vga-text.md.

    if [ "$NO_SIM" -eq 0 ]; then
        proj="VGATEXT"
        vd="$CMM_ROOT/Tests/$proj"
        vt="$TMP_DIR/$proj"
        rm -rf "$vt"; mkdir -p "$vt"
        v_fail=0
        if ! "$CMMCOMP" -o "$vt/vga_cmm" "$vd/vga_cmm.cmm" >/dev/null 2>&1 ||
           ! "$ASMCOMP" -f 25 -c 1000 "$vt/vga_cmm/vga_cmm.asm" >/dev/null 2>&1; then
            echo "FAIL ($proj): building vga_cmm"; v_fail=1
        elif ! "$CPPC" -o "$vt/vga_cpp" "$vd/vga_cpp.cpp" >/dev/null 2>&1 ||
             ! "$ASMCOMP" -f 25 -c 1000 "$vt/vga_cpp/vga_cpp.asm" >/dev/null 2>&1; then
            echo "FAIL ($proj): building vga_cpp"; v_fail=1
        elif ! "$CPPC" -o "$vt/vga_io" "$vd/vga_io.cpp" >/dev/null 2>&1 ||
             ! "$ASMCOMP" -f 25 -c 1000 "$vt/vga_io/vga_io.asm" >/dev/null 2>&1; then
            echo "FAIL ($proj): building vga_io"; v_fail=1
        else
            for p in vga_cmm vga_cpp vga_io; do
                cp "$vt/$p/${p}_data.mif" "$vt/$p/${p}_inst.mif" "$vt/$p/pc_${p}_mem.txt" "$vt/"
            done
            if ! "$IVERILOG" -g2012 -s vgatext_tb -o "$vt/v.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" "$HDL/myKeyPS2.v" "$HDL/myVGAText.v" \
                    "$vt/vga_cmm/vga_cmm.v" "$vt/vga_cpp/vga_cpp.v" "$vt/vga_io/vga_io.v" \
                    "$vd/TopLevel/vgatext_tb.v" >/dev/null 2>&1; then
                echo "FAIL ($proj): iverilog exited non-zero"; v_fail=1
            else
                vvp_capture "$proj" "$vt" "$vt/v.vvp"; v_out=$VVP_OUT
                if [ "$(printf '%s\n' "$v_out" | grep -c ': ok$')" -ne 3 ]; then
                    printf '%s\n' "$v_out" | grep "VGATEXT\|screen\|cursor" | sed 's/^/    /'
                    echo "FAIL ($proj): a system did not show the keys"; v_fail=1
                fi
            fi
        fi
        if [ $v_fail -eq 0 ]; then
            echo "PASS ($proj)  [C+- (US) and C++ (ABNT2) SAPHOs, 8 bits: keyboard -> SAPHO -> screen; C++ <iostream>, 32 bits]"
            pass=$((pass + 1))
        else
            fail=$((fail + 1)); failed_names+=("$proj")
        fi
    fi

    # ---- 3b. directed reset pass: single-cycle rst mid-run ------------------
    # ResetCheck emits one deterministic burst then parks in while(1); its
    # custom tb (TopLevel/reset_tb.v) pulses rst across exactly one posedge
    # and requires the identical burst again -- the synchronous global reset
    # must restart the processor cleanly (PC to 0, stacks rewound, racc /
    # ula_op / en_out cleared, dirty data memory rewritten by the program).

    if [ "$NO_SIM" -eq 0 ]; then
        rname="ResetCheck"
        rtest="reset"                                 # summary/failure label
        rtop="$CMM_ROOT/Tests/$rname/TopLevel"
        rtmp="$TMP_DIR/${rname}_proj"
        golden_rst="$CMM_ROOT/Tests/$rname/golden_sim"

        rm -rf "$rtmp"; mkdir -p "$rtmp"

        reset_ok=1
        if [ ! -s "$WORK_DIR/$rname/$rname/$rname.v" ]; then
            echo "FAIL ($rtest): $rname artifacts missing - did its build pass?"
            fail=$((fail + 1)); failed_names+=("$rtest"); reset_ok=0
        else
            cp "$WORK_DIR/$rname/$rname/$rname.v"          "$rtmp/"
            cp "$WORK_DIR/$rname/$rname/${rname}_data.mif" "$rtmp/"
            cp "$WORK_DIR/$rname/$rname/${rname}_inst.mif" "$rtmp/"
        fi

        if [ "$reset_ok" -eq 1 ]; then
            if ! "$IVERILOG" -s reset_tb -o "$rtmp/$rname.vvp" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" \
                    "$rtmp/$rname.v" \
                    "$rtop/reset_tb.v" >/dev/null 2>&1; then
                echo "FAIL ($rtest): iverilog exited non-zero"
                fail=$((fail + 1)); failed_names+=("$rtest"); reset_ok=0
            fi
        fi

        if [ "$reset_ok" -eq 1 ]; then
            pushd "$rtmp" >/dev/null
            "$VVP" "$rtmp/$rname.vvp" >/dev/null 2>&1
            vvp_status=$?
            if [ $vvp_status -ne 0 ]; then vvp_once_more "$rtest" "$rtmp/$rname.vvp"; vvp_status=$?; fi
            popd >/dev/null
            if [ $vvp_status -ne 0 ]; then
                echo "FAIL ($rtest): vvp exited non-zero"
                fail=$((fail + 1)); failed_names+=("$rtest"); reset_ok=0
            fi
        fi

        if [ "$reset_ok" -eq 1 ]; then
            out_f="$rtmp/output_reset.txt"
            if [ ! -s "$out_f" ]; then
                echo "FAIL ($rtest): testbench produced no output_reset.txt"
                fail=$((fail + 1)); failed_names+=("$rtest")
            else
                # Correctness anchor, independent of the golden file (stays
                # load-bearing even right after an --update): the post-reset
                # burst must repeat the boot burst line-for-line. Plain temp
                # files, not process substitution -- MSYS2 cmp cannot open the
                # /proc/<pid>/fd/N paths <(...) expands to (exit 2, ENOENT).
                # six bursts: the boot one and one after each of the tb's five
                # resets, every reset at a different phase of the spin loop
                nbursts=6
                nlines=$(grep -c '' "$out_f")
                blen=$((nlines / nbursts))
                anchor_ok=1
                if [ "$nlines" -eq 0 ] || [ $((blen * nbursts)) -ne "$nlines" ]; then
                    anchor_ok=0
                else
                    head -n "$blen" "$out_f" > "$rtmp/burst_boot.txt"
                    for k in $(seq 2 $nbursts); do
                        head -n $((blen * k)) "$out_f" | tail -n "$blen" > "$rtmp/burst_rst.txt"
                        cmp -s "$rtmp/burst_boot.txt" "$rtmp/burst_rst.txt" || anchor_ok=0
                    done
                fi
                if [ "$anchor_ok" -eq 0 ]; then
                    echo "FAIL ($rtest): a post-reset burst differs from the boot burst ($nlines lines)"
                    fail=$((fail + 1)); failed_names+=("$rtest")
                fi

                if [ "$anchor_ok" -eq 1 ]; then
                    if [ "$UPDATE" -eq 1 ]; then
                        mkdir -p "$golden_rst"
                        cp "$out_f" "$golden_rst/"
                        echo "UPDATED ($rtest)  [burst repeated after 5 one-cycle mid-run resets]"
                        pass=$((pass + 1))
                    elif [ ! -f "$golden_rst/output_reset.txt" ]; then
                        echo "FAIL ($rtest): no golden at $golden_rst (run --update?)"
                        fail=$((fail + 1)); failed_names+=("$rtest")
                    elif cmp -s "$out_f" "$golden_rst/output_reset.txt"; then
                        echo "PASS ($rtest)  [burst repeated after 5 one-cycle mid-run resets]"
                        pass=$((pass + 1))
                    else
                        echo "FAIL ($rtest): output differs from sim golden"
                        fail=$((fail + 1)); failed_names+=("$rtest")
                    fi
                fi
            fi
        fi
    fi
fi

# ---- 4. CPP phase ----------------------------------------------------------

if [ "$CMM_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CPP phase"

    shopt -s nullglob
    for entry in "$CPP_ROOT"/Tests/test*/; do
        base="$(basename "$entry")"
        # The test folder holds testN.cpp and its own files.
        src="${entry%/}/$base.cpp"
        if [ ! -f "$src" ]; then
            echo "FAIL ($base): folder layout needs $base/$base.cpp as entry"
            fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        prname="$base"                # the processor takes the source file's name

        proc="$WORK_CPP/$base"
        tmp="$WORK_CPP/$base/_tmp"   # cppcomp.log, Verilator's build
        od="$proc/$prname"           # the output folder
        rm -rf "$proc"
        mkdir -p "$tmp"

        asm="$od/$prname.asm"

        if ! "$CPPC" -o "$od" "$src" >"$tmp/cppcomp.log" 2>&1; then
            echo "FAIL ($base): cppcomp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        # warnings.txt (optional): one expected cppcomp warning per line (a
        # substring); every one must appear, and no other warning may.
        wfile="${entry%/}/warnings.txt"
        if [ -f "$wfile" ]; then
            wbad=""
            while IFS= read -r w || [ -n "$w" ]; do
                w="${w%$'\r'}"; [ -z "$w" ] && continue
                grep -qF -- "$w" "$tmp/cppcomp.log" || wbad="missing warning: $w"
            done < "$wfile"
            wexp=$(grep -c . "$wfile"); wgot=$(grep -c 'warning:' "$tmp/cppcomp.log")
            [ -z "$wbad" ] && [ "$wgot" -ne "$wexp" ] && wbad="$wgot warnings, expected $wexp"
            if [ -n "$wbad" ]; then
                echo "FAIL ($base): $wbad"; fail=$((fail+1)); failed_names+=("$base"); continue
            fi
        fi
        if ! "$ASMCOMP" -f 100 -c 5000000 "$asm" >/dev/null 2>&1; then
            echo "FAIL ($base): asmcomp"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi
        if ! hw_msg=$(hw_check "${entry%/}/hardware.txt" "$od/$prname.v"); then
            echo "FAIL ($base): $hw_msg"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        uproc="$od/$prname"
        tb="$od/${prname}_tb.v"
        if [ ! -s "$uproc.v" ] || [ ! -s "$tb" ]; then
            echo "FAIL ($base): asmcomp artifacts missing"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        out="$od/output_0.txt"
        infile="${entry%/}/$base.in"
        if [ -f "$infile" ]; then
            if [ "$NO_SIM" -eq 1 ]; then
                echo "PASS ($base)  [verilator sim skipped]"
                pass=$((pass+1)); continue
            fi
            # heavy / input-driven test: simulate with Verilator (iverilog too slow).
            # Verilator's make->g++ chain wants env-propagated TMP/TEMP, which
            # the bash->Win32 boundary sometimes strips; delegate to a
            # PowerShell helper which propagates env reliably.
            gold="${entry%/}/golden.txt"
            exp=0; [ -f "$gold" ] && exp=$(grep -c '' "$gold")
            clocks_file="${infile%.in}.clocks"
            clocks=200000000
            [ -f "$clocks_file" ] && clocks=$(cat "$clocks_file" | tr -d '[:space:]')
            # On Windows the PowerShell helper works around bash->make env
            # stripping under MSYS2; elsewhere (Linux) Verilator runs straight
            # from bash -- no env boundary, no hardcoded paths.
            if command -v powershell.exe >/dev/null 2>&1 && command -v cygpath >/dev/null 2>&1; then
                if ! powershell.exe -ExecutionPolicy Bypass -NoProfile \
                        -File "$CPP_ROOT/Tests/Verilator/run_verilator_step.ps1" \
                        -Prname  "$prname" \
                        -TmpDir  "$tmp" \
                        -UprocV  "$uproc.v" \
                        -SimMain "$SIMMAIN" \
                        -HdlDir  "$HDL" \
                        -InFile  "$infile" \
                        -OutFile "$out" \
                        -Clocks  "$clocks" \
                        -Expected "$exp" \
                        -VlBin   "$(cygpath -m "$(dirname "${VERILATOR:-/mingw64/bin/verilator}")")" \
                        -WinTmp  "$TMP" >/dev/null 2>&1; then
                    echo "FAIL ($base): verilator"; fail=$((fail+1)); failed_names+=("$base"); continue
                fi
            else
                vldir="$tmp/vl"
                if ! "$VERILATOR" --cc --exe --build --top-module "$prname" --prefix Vtop \
                        -o "sim_$prname" -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN \
                        -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE -Wno-IMPLICIT \
                        -Wno-COMBDLY --no-timing --Mdir "$vldir" \
                        "$SIMMAIN" "$uproc.v" \
                        "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" \
                        "$HDL/addr_dec.v" "$HDL/instr_dec.v" >/dev/null 2>&1 \
                   || [ ! -x "$vldir/sim_$prname" ]; then
                    echo "FAIL ($base): verilator"; fail=$((fail+1)); failed_names+=("$base"); continue
                fi
                # Output is compared separately; the sim's idle-detector can exit
                # non-zero on programs that legitimately stall between out() calls.
                # In the output folder: the processor names its .mif files relative to it.
                (cd "$od" && "$vldir/sim_$prname" "$infile" "$out" "$clocks" "$exp") >/dev/null 2>&1 || true
            fi
        else
            if ! "$IVERILOG" -s "${prname}_tb" -o "$od/$prname.vvp" \
                    "$tb" "$uproc.v" \
                    "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" \
                    "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1; then
                echo "FAIL ($base): iverilog"; fail=$((fail+1)); failed_names+=("$base"); continue
            fi

            pushd "$od" >/dev/null
            "$VVP" "$od/$prname.vvp" >/dev/null 2>&1
            vvp_status=$?
            # died: non-zero, or an empty / truncated output (a strict prefix
            # of the golden) -- once more (see vvp_once_more)
            gold_c="${entry%/}/golden.txt"
            died=0
            [ $vvp_status -ne 0 ] && died=1
            [ ! -s "$out" ] && died=1
            if [ $died -eq 0 ] && [ -f "$gold_c" ]; then
                osz=$(wc -c < "$out"); gsz=$(wc -c < "$gold_c")
                if [ "$osz" -lt "$gsz" ] && head -c "$osz" "$gold_c" | cmp -s - "$out"; then died=1; fi
            fi
            [ $died -eq 1 ] && vvp_once_more "$base" "$od/$prname.vvp"
            popd >/dev/null
            if ! stack_check "$od/${prname}_tb.vcd"; then
                echo "FAIL ($base): a stack reached its depth in the simulation (fl_full)"
                fail=$((fail+1)); failed_names+=("$base"); continue
            fi
        fi
        if [ ! -f "$out" ]; then
            echo "FAIL ($base): no simulation output"; fail=$((fail+1)); failed_names+=("$base"); continue
        fi

        gold="${entry%/}/golden.txt"
        if [ "$UPDATE" -eq 1 ]; then
            cp "$out" "$gold"
            echo "UPDATED ($base): $(tr '\n' ' ' < "$out")"
            pass=$((pass+1))
        elif [ ! -f "$gold" ]; then
            echo "FAIL ($base): no golden at $gold (run --update?)"; fail=$((fail+1)); failed_names+=("$base")
        elif cmp -s "$out" "$gold"; then
            echo "PASS ($base): $(tr '\n' ' ' < "$out")"
            pass=$((pass+1))
        else
            echo "FAIL ($base): output differs from golden"
            echo "    golden: $(tr '\n' ' ' < "$gold")"
            echo "    got:    $(tr '\n' ' ' < "$out")"
            fail=$((fail+1)); failed_names+=("$base")
        fi
    done
fi

# ---- 4a2. ISA table ---------------------------------------------------------
# The instruction set is written down in four places that nothing keeps in
# step; a disagreement between them is silent (a program assembles and then
# runs as a different program). Hold them to Compilers/common/isa.tsv.
# GitHub issue #33.
if command -v python3 >/dev/null 2>&1; then
    echo ""
    echo "==> ISA table"
    if python3 "$ROOT/Scripts/check_isa.py" "$ROOT"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("isa-table")
    fi
    # the manual's instruction page is generated from the same table
    if python3 "$ROOT/Scripts/gen_isa_doc.py" "$ROOT" --check; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("isa-doc")
    fi
    # and the manual's message reference from the compilers' sources
    if python3 "$ROOT/Scripts/gen_messages_doc.py" "$ROOT" --check; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("messages-doc")
    fi
    # and CHANGELOG.md's link lines to its version headings (issue #51)
    if python3 "$ROOT/Scripts/check_changelog.py" "$ROOT"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("changelog-links")
    fi
    # and every combination of the Dirac notation through cmmcomp
    # (Compilers/CMMComp/DiracCases/cases.tsv)
    dirac_cmm="$CMMCOMP"
    command -v cygpath >/dev/null 2>&1 && dirac_cmm=$(cygpath -w "$CMMCOMP")
    if python3 "$ROOT/Scripts/dirac_cases.py" "$ROOT" "$dirac_cmm"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("dirac-cases")
    fi
    # the sim goldens of the Dirac fixtures against values computed apart
    # from YANC: dirac_shapes exactly (Scripts/dirac_ref.py), proc_rls against
    # an RLS filter in double precision (Scripts/rls_ref.py)
    if python3 "$ROOT/Scripts/dirac_ref.py" "$ROOT" --check; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("dirac-ref")
    fi
    if python3 "$ROOT/Scripts/rls_ref.py" "$ROOT"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("rls-ref")
    fi
    # no statement of a C± test leaves a word on the data stack: the depth
    # worked out from isa.tsv is 0 at every output and at @fim (Scripts/stack_balance.py)
    if python3 "$ROOT/Scripts/stack_balance.py" "$ROOT"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("stack-balance")
    fi
    # the generated tests of Scripts/cov_tests.py: their source and golden_sim
    # are what the script computes, apart from YANC
    if python3 "$ROOT/Scripts/cov_tests.py" "$ROOT" --check; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("cov-tests")
    fi
    # how many of cmmcomp's assembly-emitting calls no test reaches, and how
    # many no test with an independent reference checks (Scripts/cmm_cov.sh,
    # Compilers/CMMComp/Tests/cov_baseline.txt): neither may grow
    if bash "$ROOT/Scripts/cmm_cov.sh" --check; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); failed_names+=("cmm-cov")
    fi
else
    echo ""
    echo "==> ISA table  [skipped: no python3]"
fi

# ---- 4a1. the exact constant encoder ------------------------------------------
# Compilers/common/yanc_num.c (TODO.md item 5) against an exact Python model:
# fixed constants, constructed ties, the denormal / overflow edges and random
# decimal text, at 32/23/8, 32/25/6, 16/10/5 and two narrow formats.
if command -v python3 >/dev/null 2>&1; then
    echo ""
    echo "==> Constant encoder (yanc_num)"
    yn_exe="$SCRATCH/yanc_num_test.exe"
    if gcc -O2 -Wall -Werror -o "$yn_exe" "$ROOT/Compilers/common/yanc_num.c" \
            "$ROOT/Compilers/common/yanc_num_test.c" >/dev/null 2>&1 \
       && python3 "$ROOT/Scripts/check_yanc_num.py" "$yn_exe" 3000; then
        pass=$((pass + 1))
    else
        echo "FAIL (yanc_num): the encoder disagrees with the exact model (or did not build)"
        fail=$((fail + 1)); failed_names+=("yanc_num")
    fi
    # the same model end to end (TODO item 5, step 5): C± brace lists of float
    # literals in every form, through cmmcomp and asmcomp, against the data
    # .mif word for word, at 32/23/8, 32/25/6, 16/10/5 and 32/23/8 #FROUND 1
    if [ "$CPP_ONLY" -eq 0 ]; then
        if python3 "$ROOT/Scripts/check_const_e2e.py" "$CMMCOMP" "$ASMCOMP" "$SCRATCH/const_e2e"; then
            pass=$((pass + 1))
        else
            echo "FAIL (const_e2e): a constant in the .mif differs from the exact model"
            fail=$((fail + 1)); failed_names+=("const_e2e")
        fi
    fi
fi

# ---- 4a0. the ALU against what each operation means ---------------------------
# Scripts/hw/tb_ula.sh drives the whole ula (every opcode on) through each
# operation number and checks every result against a reference derived in the
# testbench, never blessed; Scripts/hw/tb_alu.sh does the same
# for the shared shifter, the integer divider, F2I and the float comparison as
# blocks, in four formats. Both at every #FROUND level. Scripts/hw/tb_keyps2.sh
# checks SAPHO/myKeyPS2.v alone (both layouts, two clocks, three queue sizes);
# Scripts/hw/tb_vgatext.sh 8, SAPHO/myVGAText.v alone with an 8-bit word.
if [ "$CPP_ONLY" -eq 0 ] && [ "$NO_SIM" -eq 0 ]; then
    echo ""
    echo "==> unit testbenches (ALU, keyboard, screen)"
    for t in tb_ula tb_alu tb_keyps2 tb_vgatext; do
        t_arg=""; [ "$t" = tb_vgatext ] && t_arg=8          # one width: about 3 minutes each
        if t_out=$(bash "$ROOT/Scripts/hw/$t.sh" $t_arg 2>&1); then
            echo "PASS ($t): $(printf '%s\n' "$t_out" | grep -c ': ok') configurations"
            pass=$((pass + 1))
        else
            echo "FAIL ($t):"
            printf '%s\n' "$t_out" | grep -v ': ok' | head -12 | sed 's/^/    /'
            fail=$((fail + 1)); failed_names+=("$t")
        fi
    done
fi

# ---- 4a. every SAPHO block ---------------------------------------------------
# sapho_all is a program written to instantiate every optional block of the
# processor. Its sim golden ran in the CMM phase; here: (1) the generated .v
# really sets every opcode parameter (bar the listed exceptions) plus FFT,
# #PRACA and #TOAQUI, and (2) the sim golden is the value Tests/sapho_all/
# model.py computes from the ULA's operator definitions, so it cannot be
# re-blessed from a wrong run.
if [ "$CPP_ONLY" -eq 0 ] && command -v python3 >/dev/null 2>&1; then
    echo ""
    echo "==> SAPHO blocks (sapho_all)"
    sa="$CMM_ROOT/Tests/sapho_all"
    sa_v="$WORK_DIR/sapho_all/sapho_all/sapho_all.v"
    if [ "$NO_SIM" -eq 1 ]; then
        # --no-sim stops before asmcomp: there is no .v to check
        echo "SKIP (sapho_all-blocks): --no-sim builds no .v"
    elif [ ! -s "$sa_v" ]; then
        echo "FAIL (sapho_all-blocks): $sa_v missing - did its build pass?"
        fail=$((fail + 1)); failed_names+=("sapho_all-blocks")
    elif python3 "$ROOT/Scripts/check_blocks.py" "$sa_v" "$HDL/processor.v" "$sa/blocks_except.txt"; then
        pass=$((pass + 1))
    else
        echo "FAIL (sapho_all-blocks): a block is not instantiated (see above)"
        fail=$((fail + 1)); failed_names+=("sapho_all-blocks")
    fi
    # one output line per turn of the program's while (1): every line must be
    # the model's value
    sa_in=$(head -n 1 "$sa/input_0.txt" | tr -d '\r')
    sa_want=$(python3 "$sa/model.py" "$sa_in")
    sa_lines=$(grep -c '' "$sa/golden_sim/output_0.txt")
    sa_other=$(tr -d '\r' < "$sa/golden_sim/output_0.txt" | grep -cvx -- "$sa_want")
    if [ "$sa_lines" -gt 0 ] && [ "$sa_other" -eq 0 ]; then
        echo "PASS (sapho_all-model): all $sa_lines golden lines = model.py($sa_in) = $sa_want"
        pass=$((pass + 1))
    else
        echo "FAIL (sapho_all-model): $sa_other of $sa_lines golden lines differ from model.py($sa_in) = $sa_want"
        fail=$((fail + 1)); failed_names+=("sapho_all-model")
    fi
fi

# ---- 4a2b. round to nearest on varying data -----------------------------------
# cmm_fround_mean accumulates pseudo-random addends at #FROUND 2 (TODO item
# 2(b)). Its sim golden is checked line by line against model.py, which rounds
# by the definition (nearest, ties to even), so a wrong run cannot be blessed.
# The golden holds several turns of the while (1); a turn is 32 lines.
if [ "$CPP_ONLY" -eq 0 ] && command -v python3 >/dev/null 2>&1; then
    fm="$CMM_ROOT/Tests/cmm_fround_mean"
    fm_want="$SCRATCH/fround_mean_model.txt"
    python3 "$fm/model.py" 2 | tr -d '\r' > "$fm_want"
    fm_lines=$(grep -c '' "$fm/golden_sim/output_0.txt")
    fm_other=$(tr -d '\r' < "$fm/golden_sim/output_0.txt" \
        | awk 'NR==FNR { m[FNR-1] = $0; n = FNR; next } $0 != m[(FNR-1) % n] { bad++ } END { print bad+0 }' "$fm_want" -)
    if [ "$fm_lines" -gt 0 ] && [ "$fm_other" -eq 0 ]; then
        echo "PASS (fround_mean-model): all $fm_lines golden lines = model.py 2 (final error $(tail -n 1 "$fm_want") / 64 ULP)"
        pass=$((pass + 1))
    else
        echo "FAIL (fround_mean-model): $fm_other of $fm_lines golden lines differ from model.py 2"
        fail=$((fail + 1)); failed_names+=("fround_mean-model")
    fi
fi

# ---- 4a3. a float or a complex as a condition -------------------------------
# Scripts/fcond: the same cases (if, while, for, do, !, &&, ||, ?:, bool) on
# the host compiler, which gives C's answer, and on cppcomp and cmmcomp at
# three float formats and the three #FROUND levels, simulated; every 0/1
# must agree. Needs the simulator and python3.
if command -v python3 >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1; then
    echo ""
    echo "==> Float and complex conditions against the host C compiler (fcond)"
    if [ "$NO_SIM" -eq 1 ]; then
        echo "SKIP (float-conditions): --no-sim"
    elif BIN="$BIN_DIR" HDL="$HDL" SHORT=1 bash "$ROOT/Scripts/fcond/run.sh" "$WORK_DIR/fcond" > "$WORK_DIR/fcond.log" 2>&1; then
        echo "PASS (float-conditions)"
        pass=$((pass + 1))
    else
        echo "FAIL (float-conditions): see $WORK_DIR/fcond.log"
        grep -E "differ|BUILD FAILED" "$WORK_DIR/fcond.log" | grep -v " 0 differ" | head -10 | sed 's/^/    /'
        fail=$((fail + 1)); failed_names+=("float-conditions")
    fi
else
    echo "==> Float and complex conditions  [skipped: no python3 or g++]"
fi

# ---- 4b. CMM negative phase (error reporting) ------------------------------
# Error-path coverage the golden phases lack: each fixture is a malformed
# program that cmmcomp must REJECT with a clean non-zero exit and the right
# diagnostic -- never a crash, never a silent accept. Fixtures live in
# NegTests/ (a sibling of Tests/) so the CI smoke, which walks Tests/*/,
# never tries to compile them.
if [ "$CPP_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CMM negative phase (error reporting)"
    neg_dir="$CMM_ROOT/NegTests"
    neg_work="$SCRATCH/work_neg"
    while IFS='|' read -r fixture expect; do
        fixture="${fixture%$'\r'}"; expect="${expect%$'\r'}"   # tolerate CRLF manifest
        case "$fixture" in ''|\#*) continue ;; esac            # skip blanks/comments
        name="neg:${fixture%.cmm}"
        if [ ! -f "$neg_dir/$fixture" ]; then
            echo "FAIL ($name): fixture missing ($neg_dir/$fixture)"
            fail=$((fail+1)); failed_names+=("$name"); continue
        fi
        rm -rf "$neg_work"; mkdir -p "$neg_work"
        cp "$neg_dir/$fixture" "$neg_work/$fixture"   # its own name: the processor's
        out="$("$CMMCOMP" -o "$neg_work/p" "$neg_work/$fixture" 2>&1)"
        rc=$?
        if [ "$rc" -eq 0 ]; then
            echo "FAIL ($name): compiled (exit 0) but should have been rejected"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ "$rc" -ge 128 ]; then
            echo "FAIL ($name): crashed (exit $rc) instead of a clean error"
            fail=$((fail+1)); failed_names+=("$name")
        elif ! printf '%s' "$out" | grep -qF "$expect"; then
            echo "FAIL ($name): exit $rc but missing diagnostic '$expect'"
            echo "    got: $(printf '%s' "$out" | head -1)"
            fail=$((fail+1)); failed_names+=("$name")
        else
            echo "PASS ($name): rejected (exit $rc) -- \"$expect\""
            pass=$((pass+1))
        fi
    done < "$neg_dir/manifest.txt"
    # the reverse: a valid program that must compile WITHOUT a given message
    # (a spurious warning), format <fixture.cmm>|<forbidden substring>; or
    # WITH one (a warning that must fire), format <fixture.cmm>|+<substring>
    while IFS='|' read -r fixture forbid; do
        fixture="${fixture%$'\r'}"; forbid="${forbid%$'\r'}"
        case "$fixture" in ''|\#*) continue ;; esac
        name="warn:${fixture%.cmm}"
        want=0; case "$forbid" in +*) want=1; forbid="${forbid#+}" ;; esac
        rm -rf "$neg_work"; mkdir -p "$neg_work"
        cp "$neg_dir/$fixture" "$neg_work/p.cmm"
        out="$("$CMMCOMP" -o "$neg_work/p" "$neg_work/p.cmm" 2>&1)"
        rc=$?
        printed=0; printf '%s' "$out" | grep -qF "$forbid" && printed=1
        if [ "$rc" -ne 0 ]; then
            echo "FAIL ($name): rejected (exit $rc), should compile"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ "$printed" -ne "$want" ]; then
            if [ "$want" -eq 1 ]; then echo "FAIL ($name): did not print '$forbid'"
            else                       echo "FAIL ($name): printed '$forbid'"; fi
            fail=$((fail+1)); failed_names+=("$name")
        else
            if [ "$want" -eq 1 ]; then echo "PASS ($name): printed \"$forbid\""
            else                       echo "PASS ($name): compiled without \"$forbid\""; fi
            pass=$((pass+1))
        fi
    done < "$neg_dir/nowarn.txt"
    rm -rf "$neg_work"
fi

# ---- 4b2. float format refused by asmcomp -----------------------------------
# cmmcomp accepts these two; asmcomp must stop on the float format (TODO item
# 3), checked only because they use float (Tests/cmm_nofloat is the other side:
# no float, a format that does not add up, accepted).
if [ "$CPP_ONLY" -eq 0 ]; then
    echo ""
    echo "==> float format (asmcomp)"
    neg_work="$SCRATCH/work_neg"
    for fe in "fpfmt_sum|doesn't add up" "fpfmt_expo|exponent is too short" "fpfmt_wide|is past 32"; do
        fixture="${fe%%|*}"; expect="${fe#*|}"; name="asm:$fixture"
        rm -rf "$neg_work"; mkdir -p "$neg_work/p"
        if [ -f "$CMM_ROOT/NegTests/$fixture.asm" ]; then     # written by hand: asmcomp alone
            cp "$CMM_ROOT/NegTests/$fixture.asm" "$neg_work/p/"
        else
            cp "$CMM_ROOT/NegTests/$fixture.cmm" "$neg_work/"
            if ! "$CMMCOMP" -o "$neg_work/p" "$neg_work/$fixture.cmm" >/dev/null 2>&1; then
                echo "FAIL ($name): cmmcomp rejected it, the check is asmcomp's"
                fail=$((fail+1)); failed_names+=("$name"); continue
            fi
        fi
        out="$("$ASMCOMP" "$neg_work/p/$fixture.asm" 2>&1)"
        rc=$?
        if [ "$rc" -eq 0 ]; then
            echo "FAIL ($name): asmcomp accepted it"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ "$rc" -ge 128 ] || ! printf '%s' "$out" | grep -qF "$expect"; then
            echo "FAIL ($name): exit $rc, missing '$expect'"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ -e "$neg_work/p/$fixture.v" ]; then
            echo "FAIL ($name): refused, but the .v was written"
            fail=$((fail+1)); failed_names+=("$name")
        else
            echo "PASS ($name): refused (exit $rc) -- \"$expect\""
            pass=$((pass+1))
        fi
    done
    rm -rf "$neg_work"
fi

# ---- 4c. CPP negative phase (error reporting) ------------------------------
# The same for cppcomp: Compilers/CPPComp/NegTests/manifest.txt, format
# <fixture.cpp>|<expected substring>[|<lang>], <lang> en (default) or pt.
if [ "$CMM_ONLY" -eq 0 ]; then
    echo ""
    echo "==> CPP negative phase (error reporting)"
    neg_dir="$CPP_ROOT/NegTests"
    neg_work="$SCRATCH/work_cppneg"
    while IFS='|' read -r fixture expect lang; do
        fixture="${fixture%$'\r'}"; expect="${expect%$'\r'}"; lang="${lang%$'\r'}"
        case "$fixture" in ''|\#*) continue ;; esac
        name="cppneg:${fixture%.cpp}"
        if [ ! -f "$neg_dir/$fixture" ]; then
            echo "FAIL ($name): fixture missing ($neg_dir/$fixture)"
            fail=$((fail+1)); failed_names+=("$name"); continue
        fi
        rm -rf "$neg_work"; mkdir -p "$neg_work/tmp"
        out="$(YANC_LANG="${lang:-en}" "$CPPC" -o "$neg_work/p" "$neg_dir/$fixture" 2>&1)"
        rc=$?
        if [ "$rc" -eq 0 ]; then
            echo "FAIL ($name): compiled (exit 0) but should have been rejected"
            fail=$((fail+1)); failed_names+=("$name")
        elif [ "$rc" -ge 128 ]; then
            echo "FAIL ($name): crashed (exit $rc) instead of a clean error"
            fail=$((fail+1)); failed_names+=("$name")
        elif ! printf '%s' "$out" | grep -qF "$expect"; then
            echo "FAIL ($name): exit $rc but missing diagnostic '$expect'"
            echo "    got: $(printf '%s' "$out" | head -1)"
            fail=$((fail+1)); failed_names+=("$name")
        else
            echo "PASS ($name): rejected (exit $rc) -- \"$expect\""
            pass=$((pass+1))
        fi
    done < "$neg_dir/manifest.txt"
    rm -rf "$neg_work"
fi

# ---- 4d. the driver, yanc --------------------------------------------------
# yanc runs the tools next to it: on a C+- test its .asm must be the golden;
# -S then the .asm must give the same processor; on a C++ test its .asm must
# be cppcomp's own; a bad input and a failing compile must stop it non-zero;
# a program in a folder with accents and spaces simulates from its output folder;
# an .asm with no compiler tables beside it assembles and simulates.
echo ""
echo "==> yanc (the driver)"
drv="$SCRATCH/work_yanc"
rm -rf "$drv"; mkdir -p "$drv"
cp -r "$CMM_ROOT/Tests/sw_test" "$drv/sw_test"
cp -r "$CPP_ROOT/Tests/test1"   "$drv/test1"
( cd "$drv/sw_test" && "$YANC" sw_test.cmm >/dev/null 2>&1 ) && [ -f "$drv/sw_test/sw_test/sw_test.v" ] \
    && diff -q "$drv/sw_test/golden.asm" "$drv/sw_test/sw_test/sw_test.asm" >/dev/null
r1=$?
( cd "$drv/sw_test" && rm -rf sw_test && "$YANC" -S sw_test.cmm >/dev/null 2>&1 && [ ! -f sw_test/sw_test.v ] \
    && "$YANC" sw_test/sw_test.asm >/dev/null 2>&1 ) && [ -f "$drv/sw_test/sw_test/sw_test.v" ]
r2=$?
( cd "$drv/test1" && "$YANC" -o by_yanc test1.cpp >/dev/null 2>&1 && "$CPPC" -o direct test1.cpp >/dev/null 2>&1 ) \
    && diff -q "$drv/test1/by_yanc/test1.asm" "$drv/test1/direct/test1.asm" >/dev/null && [ -f "$drv/test1/by_yanc/test1.v" ]
r3=$?
printf 'void main()\n{\n    int a = 1\n}\n' > "$drv/bad.cmm"
cp "$CMM_ROOT/Tests/cmm_bigin/golden.asm" "$drv/bad-name.asm"   # an .asm names the processor too
( cd "$drv" && ! "$YANC" bad.cmm >/dev/null 2>&1 && ! "$YANC" bad.txt >/dev/null 2>&1 \
    && ! "$YANC" bad-name.asm >/dev/null 2>&1 && [ ! -f bad-name.v ] )
r4=$?
# a folder with accents and spaces: the tools work in UTF-8 on every system,
# the Verilog names its files relative to the output folder (no absolute
# path), and the simulation, run there, reads ../input_0.txt
acc="$drv/Pasta Ação ü"
mkdir -p "$acc"
cp "$CMM_ROOT/Tests/cmm_bigin/cmm_bigin.cmm" "$acc/eco.cmm"
cp "$CMM_ROOT/Tests/cmm_bigin/input_0.txt"   "$acc/"
( cd "$acc" && "$YANC" eco.cmm >/dev/null 2>&1 ) && [ -f "$acc/eco/eco.v" ] \
    && grep -q '^srcdir .*/Pasta Ação ü$' "$acc/eco/prog_log.txt" \
    && ! grep -E '(fopen|readmem[bh]|FILE)\("(/|[A-Za-z]:)' "$acc/eco/eco.v" "$acc/eco/eco_tb.v" >/dev/null \
    && grep -q '"\.\./input_0\.txt"' "$acc/eco/eco_tb.v"
r5=$?
if [ "$r5" -eq 0 ] && [ "$NO_SIM" -eq 0 ]; then
    ( cd "$acc/eco" && "$IVERILOG" -s eco_tb -o eco.vvp eco_tb.v eco.v \
          "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1 \
      && "$VVP" eco.vvp >/dev/null 2>&1 ) \
        && cmp -s "$acc/eco/output_0.txt" "$CMM_ROOT/Tests/cmm_bigin/golden_sim/output_0.txt"
    r5=$?
fi
# an .asm on its own (written by hand: no prog_log.txt beside it, no
# #PRNAME line): it assembles with exit 0, its file name names the
# processor, its testbench reads input_0.txt from the same folder, and it
# simulates to the golden
hand="$drv/hand"
mkdir -p "$hand"
grep -v '^#PRNAME' "$CMM_ROOT/Tests/cmm_bigin/golden.asm" > "$hand/eco_hand.asm"
cp "$CMM_ROOT/Tests/cmm_bigin/input_0.txt" "$hand/"
( cd "$drv" && "$YANC" hand/eco_hand.asm >/dev/null 2>&1 ) && [ -f "$hand/eco_hand_tb.v" ]
r6=$?
if [ "$r6" -eq 0 ] && [ "$NO_SIM" -eq 0 ]; then
    ( cd "$hand" && "$IVERILOG" -s eco_hand_tb -o h.vvp eco_hand_tb.v eco_hand.v \
          "$HDL/addr_dec.v" "$HDL/instr_dec.v" "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" >/dev/null 2>&1 \
      && "$VVP" h.vvp >/dev/null 2>&1 ) \
        && cmp -s "$hand/output_0.txt" "$CMM_ROOT/Tests/cmm_bigin/golden_sim/output_0.txt"
    r6=$?
fi
for t in "1 C+- to Verilog, .asm = golden" "2 -S then the .asm" "3 C++ .asm = cppcomp's" "4 errors stop it" "5 a folder with accents and spaces" "6 an .asm on its own"; do
    n="${t%% *}"; eval "r=\$r$n"
    if [ "$r" -eq 0 ]; then echo "PASS (yanc:${t#* })"; pass=$((pass+1))
    else echo "FAIL (yanc:${t#* })"; fail=$((fail+1)); failed_names+=("yanc$n"); fi
done
rm -rf "$drv"

# ---- 5. summary ------------------------------------------------------------

echo ""
if [ "${#retried_names[@]}" -gt 0 ]; then
    echo "retried (simulation died, likely short of memory): $(printf '%s ' "${retried_names[@]}")"
fi
echo "===== $pass passed, $fail failed ====="
if [ "$fail" -ne 0 ]; then
    echo "failed: $(printf '%s ' "${failed_names[@]}")"
    exit 1
fi

# write the new baseline if --update-size was requested
if [ "$UPDATE_SIZE" -eq 1 ]; then
    {
        echo "# num_ins ratchet: each line is \"<prname> <count>\"."
        echo "# Captures the number of real instructions cmmcomp emits per example"
        echo "# (excludes labels, directives and macros). regress.sh fails any run where"
        echo "# a count grows or an example has no line. To update after a refactor that shrinks"
        echo "# things, run: Scripts/regress.sh --update-size"
        # bash's `set -u` raises "unbound variable" on ${assoc[$key]} even
        # when the key is in ${!assoc[@]} (seen on bash 5.2). Build the
        # output lines first, then sort, then print.
        if [ "${#SIZE_CURRENT[@]}" -gt 0 ]; then
            set +u
            lines=""
            for name in "${!SIZE_CURRENT[@]}"; do
                lines+=$(printf "%-10s %s" "$name" "${SIZE_CURRENT[$name]}")$'\n'
            done
            set -u
            printf "%s" "$lines" | sort
        fi
    } > "$SIZE_BASELINE_FILE"
    echo ""
    echo "===== size baseline rewritten ($SIZE_BASELINE_FILE) ====="
fi
exit 0
