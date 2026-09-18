#!/bin/bash
# LUT4 area and critical-path depth of HDL/ula.v per operator set (Yosys).
#
# Usage:  [NOSHARE=1] [NUBITS=.. NBMANT=.. NBEXPO=..] [NUGAIN=..] bash Scripts/hw/area.sh <fround> <config>...
# e.g.    NOSHARE=1 bash Scripts/hw/area.sh 2 fadd fmlt all_nodiv
#
# ALWAYS pass NOSHARE=1 for numbers to compare: Yosys' `synth` runs SAT-based
# resource sharing that merges mutually exclusive operators (F2I's shifters
# with the normaliser's), inflating depth by ~7 levels. Quartus does not.
# And `abc` is heuristic: deltas under ~10 % are noise (measured: 4209 vs 3827
# LUT4 on a netlist Quartus proved identical). Quartus is the reference.
#
# Results are appended to <scratch>/area.txt and echoed.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${OUT:-$ROOT/.smoke/hw}
mkdir -p "$OUT" || exit 1
FR=$1; shift || { echo "usage: area.sh <fround> <config>..."; exit 1; }

# Yosys 0.56 asserts when a part-select feeds a parameterised instance, which
# is how ula.v wires I2F. Route those through named wires in a working copy.
# Yosys is a native Windows build: it does not understand MSYS paths like
# /c/..., so everything below runs with the scratch dir as cwd and plain
# relative file names.
ULA=ula_area.v
sed 's|my_i2f (in2, i2f)|my_i2f (in2_w, i2f)|; s|my_i2fm(in1, i2fm)|my_i2fm(in1_w, i2fm)|; s|^// I2F ------|wire signed [NUBITS-1:0] in2_w = in2; wire signed [NUBITS-1:0] in1_w = in1; // yosys part-select workaround\n// I2F ------|' "$ROOT/HDL/ula.v" > "$OUT/$ULA"

INT='ADD MLT DIV MOD NEG ABS AND ORR XOR INV LAN LOR LIN LES GRE EQU SHL SHR SRS'
INT_NODIV='ADD MLT NEG ABS AND ORR XOR INV LAN LOR LIN LES GRE EQU SHL SHR SRS'
FLT='F_ADD F_SU1 F_SU2 F_MLT F_DIV I2F F2I F_NEG F_ABS F_LES F_GRE F_SGN'
FLT_NODIV='F_ADD F_SU1 F_SU2 F_MLT I2F F2I F_NEG F_ABS F_LES F_GRE F_SGN'

run () {
    local label=$1; shift
    local tag="${NUBITS:-32}_${FR}_$label${NOSHARE:+_ns}"
    local cp="-chparam FROUND $FR -chparam NUBITS ${NUBITS:-32} -chparam NBMANT ${NBMANT:-23} -chparam NBEXPO ${NBEXPO:-8}"
    # NUGAIN=<n> overrides the norm() divisor (a non-power-of-two infers a full divider in ula_nrm)
    [ -n "${NUGAIN:-}" ] && cp="$cp -chparam NUGAIN $NUGAIN"
    local p; for p in "$@"; do cp="$cp -chparam $p 1"; done
    cat > "$OUT/$tag.ys" <<YS
read_verilog -sv $ULA
hierarchy -top ula $cp
synth -top ula -flatten ${NOSHARE:+-noshare}
abc -lut 4
opt_clean
stat
ltp -noff
YS
    ( cd "$OUT" && yosys -q -l "$tag.log" "$tag.ys" >/dev/null 2>&1 )
    local luts depth
    luts=$(grep -E '^\s+\$lut\s+[0-9]+' "$OUT/$tag.log" | tail -1 | awk '{print $2}')
    depth=$(grep -oE 'length=[0-9]+' "$OUT/$tag.log" | tail -1 | cut -d= -f2)
    if [ -z "$luts" ]; then echo "${NUBITS:-32}/${NBMANT:-23}/${NBEXPO:-8} FROUND=$FR/$label: FAILED (see $OUT/$tag.log)"; return 1; fi
    echo "${NUBITS:-32}/${NBMANT:-23}/${NBEXPO:-8} FROUND=$FR/$label${NOSHARE:+ (noshare)}: LUT4=$luts depth=$depth" | tee -a "$OUT/area.txt"
}

for cfg in "$@"; do
    case $cfg in
        fadd)       run fadd      F_ADD F_SU1 F_SU2 ;;
        fmlt)       run fmlt      F_MLT ;;
        fdiv)       run fdiv      F_DIV ;;
        fcmp)       run fcmp      F_LES F_GRE ;;
        i2f)        run i2f       I2F ;;
        f2i)        run f2i       F2I ;;
        idiv)       run idiv      DIV MOD ;;
        nrm)        run nrm${NUGAIN:+_g$NUGAIN} NRM NRM_M ;;
        div)        run div       DIV ;;
        mod)        run mod       MOD ;;
        shift)      run shift     SHL SHR SRS ;;
        shl)        run shl       SHL ;;
        shr)        run shr       SHR ;;
        srs)        run srs       SRS ;;
        imlt)       run imlt      MLT ;;
        int)        run int       $INT ;;
        int_nodiv)  run int_nodiv $INT_NODIV ;;
        flt)        run flt       $FLT ;;
        flt_nodiv)  run flt_nodiv $FLT_NODIV ;;
        all)        run all       $INT $FLT ;;
        all_nofdiv) run all_nofdiv $INT $FLT_NODIV ;;
        all_nodiv)  run all_nodiv $INT_NODIV $FLT_NODIV ;;
        *) echo "unknown config: $cfg (fadd fmlt fdiv fcmp i2f f2i idiv div mod nrm shift shl shr srs imlt int int_nodiv flt flt_nodiv all all_nofdiv all_nodiv)"; exit 1 ;;
    esac
done
