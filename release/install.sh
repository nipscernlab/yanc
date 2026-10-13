#!/bin/sh
# Puts the tools of this package on your PATH: one small launcher per tool in
# ~/.local/bin (or $PREFIX/bin), each running the real one here, so a terminal
# finds yanc, cmmcomp, cppcomp, asmcomp, ... by name and the tools keep finding
# SAPHO/, Macros/ and Header/ next to bin/.
#
#   ./install.sh                     # into ~/.local/bin
#   PREFIX=/usr/local ./install.sh   # into /usr/local/bin (may need sudo)
#   ./install.sh --uninstall         # remove the launchers
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
PREFIX=${PREFIX:-$HOME/.local}
DEST=$PREFIX/bin
mkdir -p "$DEST"
for t in yanc cmmcomp cppcomp asmcomp gen_gtkw comp2gtkw; do
    if [ "$1" = "--uninstall" ]; then
        rm -f "$DEST/$t"
    else
        printf '#!/bin/sh\n# YANC launcher, written by %s\nexec "%s" "$@"\n' "$HERE/install.sh" "$HERE/bin/$t" > "$DEST/$t"
        chmod +x "$DEST/$t"
    fi
done
if [ "$1" = "--uninstall" ]; then
    echo "removed the YANC launchers from $DEST"
    exit 0
fi
echo "YANC tools on $DEST"
case ":$PATH:" in
    *":$DEST:"*) echo "try: yanc -h" ;;
    *) echo "$DEST is not on your PATH: add  export PATH=\"$DEST:\$PATH\"  to ~/.profile" ;;
esac
