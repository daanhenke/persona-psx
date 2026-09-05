"""Prepare a decomp-permuter scratch directory for one function.

decomp-permuter compares OBJECT files, so the target object and the compiled
candidate must agree on symbol names. splat now emits plain D_<addr> /
func_<addr> names (symbol_name_format: $VRAM) which is exactly what the C
sources declare, so the two line up.

Creates permuter/<target>/<symbol>/ containing:
    base.c        preprocessed candidate, single function
    target.o      the original function assembled on its own
    compile.sh    cpp -> cc1 -> maspsx -> as
    settings.toml func_name + compiler_type

Then run:  tools/decomp-permuter/permuter.py permuter/<target>/<symbol> -j
"""
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mfunc import (ROOT, GAME, VENV, ASFLAGS, AS, FUNC_HDR, load_target, run,
                   compile_c, c_symbol, normalize_asm, load_names, gp_base,
                   target_defines)

COMPILE_SH = """#!/bin/bash
# Invoked by decomp-permuter as: ./compile.sh input.c -o output.o
set -e
ROOT={root}
INPUT="$1"; shift
OUT=""
while [ $# -gt 0 ]; do
  case "$1" in
    -o) OUT="$2"; shift 2 ;;
    *)  shift ;;
  esac
done
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

mipsel-linux-gnu-gcc -E -nostdinc -undef -D__GNUC__=2 {defines} \\
    -I "$ROOT/include" -I "$ROOT/include/psyq" "$INPUT" -o "$TMP/x.i"
"$ROOT/bin/cc1-psx-26/cc1-psx-26" {cc1flags} "$TMP/x.i" -o "$TMP/x.s"
"$ROOT/.venv/bin/python3" "$ROOT/tools/maspsx/maspsx.py" \\
    --expand-div --aspsx-version=2.34 < "$TMP/x.s" > "$TMP/x2.s"
mipsel-linux-gnu-as {asflags} -o "$OUT" "$TMP/x2.s"
"""

SETTINGS = """func_name = "{sym}"
compiler_type = "gcc"
"""

CC1FLAGS = ("-quiet -O2 -G0 -mcpu=3000 -msoft-float -mgas -gcoff -fgnu-linker "
            "-funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return "
            "-fcommon -fverbose-asm -w")


def setup(target, symbol, cfile):
    vram, orig, lines = load_target(target, symbol)
    outdir = os.path.join(ROOT, "permuter", target, symbol)
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)

    # Which symbol does the candidate actually define?
    tmp = os.path.join(outdir, "_probe")
    os.makedirs(tmp)
    names = load_names(target)
    sym = c_symbol(compile_c(os.path.abspath(cfile), tmp, target),
                   names.get(vram)) or symbol
    shutil.rmtree(tmp)

    # base.c: preprocessed, so pycparser can read it.
    base_c = os.path.join(outdir, "base.c")
    run(["mipsel-linux-gnu-gcc", "-E", "-P", "-nostdinc", "-undef",
         "-D__GNUC__=2", "-D__attribute__(x)=",
         "-I", os.path.join(ROOT, "include"),
         "-I", os.path.join(ROOT, "include", "psyq"),
         os.path.abspath(cfile), "-o", base_c])

    # target.o: the original function alone, under the candidate's symbol name.
    tgt_s = os.path.join(outdir, "target.s")
    with open(tgt_s, "w", newline="\n") as f:
        f.write(FUNC_HDR)
        f.write(".global %s\n%s:\n" % (sym, sym))
        f.write("\n".join(normalize_asm(lines, names, gp_base(target))) + "\n")
    run([AS] + ASFLAGS + ["-o", os.path.join(outdir, "target.o"), tgt_s])

    sh = os.path.join(outdir, "compile.sh")
    with open(sh, "w", newline="\n") as f:
        f.write(COMPILE_SH.format(root=ROOT, cc1flags=CC1FLAGS,
                                  defines=" ".join(target_defines(target)),
                                  asflags=" ".join(ASFLAGS)))
    os.chmod(sh, 0o755)

    with open(os.path.join(outdir, "settings.toml"), "w", newline="\n") as f:
        f.write(SETTINGS.format(sym=sym))

    # Sanity check: the scratch must build before the permuter can use it.
    probe = os.path.join(outdir, "base.o")
    r = subprocess.run([sh, base_c, "-o", probe],
                       capture_output=True, text=True, cwd=outdir)
    ok = r.returncode == 0
    print("%s/%s -> %s" % (target, symbol, os.path.relpath(outdir, ROOT)))
    print("   symbol=%s  vram=0x%08X  %d insn  compile.sh=%s"
          % (sym, vram, len(orig) // 4, "ok" if ok else "FAILED"))
    if not ok:
        print((r.stderr or r.stdout).strip()[:600])
        return 1
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit("usage: setup_permuter.py <target> <symbol> <candidate.c>")
    sys.exit(setup(sys.argv[1], sys.argv[2], sys.argv[3]))
