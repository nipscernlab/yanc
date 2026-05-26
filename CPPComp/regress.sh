#!/usr/bin/env bash
# CPPComp/regress.sh - thin shim that delegates to the unified
# Scripts/regress.sh, forwarding flags and running only the CPP phase.
# Kept for convenience: anyone with the habit of `bash CPPComp/regress.sh`
# still gets the CPPComp tests.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec bash "$SCRIPT_DIR/../Scripts/regress.sh" --cpp-only "$@"
