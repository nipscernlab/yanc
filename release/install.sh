#!/bin/sh
# Puts the tools of this package on your PATH, so that a terminal finds yanc,
# cmmcomp, cppcomp, asmcomp, ... by name, and the tools keep finding SAPHO/,
# Macros/ and Header/ next to bin/. Then checks what simulating and looking at
# the waveform take, and says how to get what is missing.
#
#   On Linux: one small launcher per tool in ~/.local/bin (or $PREFIX/bin),
#   each running the real one here.
#   In MSYS2's terminal on Windows: /etc/profile.d/yanc.sh, which puts this
#   package's bin/ on the PATH of every new MSYS2 terminal; pacman installs
#   what is missing, if you say so.
#
#   ./install.sh                     # install
#   PREFIX=/usr/local ./install.sh   # Linux: into /usr/local/bin (may need sudo)
#   ./install.sh --uninstall         # take it off again
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
case "$(uname -s)" in
    MINGW*|MSYS*|UCRT*|CLANG*) MSYS2=1 ;;
    *)                         MSYS2=0 ;;
esac

if [ "$MSYS2" = 1 ]; then
    PROFILE=/etc/profile.d/yanc.sh
    if [ "${1:-}" = "--uninstall" ]; then
        rm -f "$PROFILE"
        echo "removed $PROFILE"
        exit 0
    fi
    # After MSYS2's own folders, so that nothing here shadows them.
    if ! printf '# YANC on the PATH, written by %s\nexport PATH="$PATH:%s"\n' \
            "$HERE/install.sh" "$HERE/bin" > "$PROFILE" 2>/dev/null; then
        echo "cannot write $PROFILE; add this line to ~/.bashrc instead:"
        echo "  export PATH=\"\$PATH:$HERE/bin\""
        exit 1
    fi
    echo "YANC tools: $HERE/bin, on the PATH of every new MSYS2 terminal ($PROFILE)"
else
    DEST=${PREFIX:-$HOME/.local}/bin
    mkdir -p "$DEST"
    for t in yanc cmmcomp cppcomp asmcomp gen_gtkw comp2gtkw; do
        if [ "${1:-}" = "--uninstall" ]; then
            rm -f "$DEST/$t"
        else
            printf '#!/bin/sh\n# YANC launcher, written by %s\nexec "%s" "$@"\n' "$HERE/install.sh" "$HERE/bin/$t" > "$DEST/$t"
            chmod +x "$DEST/$t"
        fi
    done
    if [ "${1:-}" = "--uninstall" ]; then
        echo "removed the YANC launchers from $DEST"
        exit 0
    fi
    echo "YANC tools on $DEST"
    case ":$PATH:" in
        *":$DEST:"*) ;;
        *) echo "$DEST is not on your PATH: add  export PATH=\"$DEST:\$PATH\"  to ~/.profile" ;;
    esac
fi

# What the Quick start takes: Icarus to simulate, GTKWave to look at the
# waveform. Verilator, the faster simulator, is asked for apart.
missing=""
for t in iverilog vvp gtkwave; do
    if command -v "$t" >/dev/null 2>&1; then echo "[found]   $t: $(command -v "$t")"; else missing="$missing $t"; fi
done
vl_missing=""
for t in verilator make g++ python3; do
    command -v "$t" >/dev/null 2>&1 || vl_missing="$vl_missing $t"
done
if [ "$MSYS2" = 1 ] && [ ! -f "${MINGW_PREFIX:-/ucrt64}/include/lz4.h" ]; then
    vl_missing="$vl_missing lz4"
fi
[ -z "$vl_missing" ] && echo "[found]   verilator: $(command -v verilator)"

if [ "$MSYS2" = 1 ]; then
    if [ "${MSYSTEM:-}" != UCRT64 ]; then
        echo "note: this is MSYS2's ${MSYSTEM:-MSYS} terminal; YANC's simulators come from UCRT64:"
        echo "      open the \"MSYS2 UCRT64\" terminal and run ./install.sh there."
        exit 0
    fi
    P=${MINGW_PACKAGE_PREFIX:-mingw-w64-ucrt-x86_64}
    pk=""
    case "$missing" in *iverilog*|*vvp*) pk="$pk $P-iverilog" ;; esac
    case "$missing" in *gtkwave*) pk="$pk $P-gtkwave" ;; esac
    if [ -n "$pk" ]; then
        echo "missing:$missing"
        printf "install them now with  pacman -S%s ? [y/N] " "$pk"
        ans=""; read -r ans || ans=""
        case "$ans" in y|Y) pacman -S --needed --noconfirm $pk ;; esac
    fi
    if [ -n "$vl_missing" ]; then
        vk="$P-verilator $P-lz4 make $P-gcc $P-python"
        echo "Verilator, the faster simulator (optional), lacks:$vl_missing"
        printf "install it now with  pacman -S %s ? [y/N] " "$vk"
        ans=""; read -r ans || ans=""
        case "$ans" in y|Y) pacman -S --needed --noconfirm $vk ;; esac
    fi
    echo "Open a NEW UCRT64 terminal and type: yanc -h"
else
    [ -n "$missing" ] && echo "missing:$missing  (install iverilog and gtkwave with your package manager, e.g. sudo apt install iverilog gtkwave)"
    [ -n "$vl_missing" ] && echo "Verilator, the faster simulator (optional), lacks:$vl_missing  (e.g. sudo apt install verilator make g++ python3)"
    case ":$PATH:" in
        *":$DEST:"*) echo "try: yanc -h" ;;
    esac
fi
