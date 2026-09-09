"""Prepare a decomp-permuter scratch directory for one routine.

decomp-permuter compares object files, so the original and the candidate have
to agree on symbol names. splat writes plain names here (symbol_name_format:
$VRAM) and the C declares the same ones, so the two line up already.

Creates permuter/<target>/<Symbol>/ holding:
    base.c        the unit, already preprocessed - build/nm's .sjis.i
    target.o      the original routine assembled on its own
    compile.sh    cc1 -> maspsx -> as, the flags the Makefile uses
    settings.toml the routine to permute

  usage: setup_permuter.py <target> <object-under-src> <Symbol>
e.g.    setup_permuter.py btlp btlp/talkscore BtlTalkScoreLine

Then:   tools/decomp-permuter/permuter.py permuter/<target>/<Symbol> -j8
"""
import os
import shutil
import subprocess
import sys

ROOT = '/mnt/g/projects/persona-psx-decomp'

# exactly what the Makefile hands cc1 and maspsx for an overlay unit
CC1FLAGS = ('-O2 -G0 -mips1 -mcpu=3000 -w -funsigned-char -fpeephole '
            '-ffunction-cse -fpcc-struct-return -fcommon -fverbose-asm '
            '-msoft-float -mgas -fgnu-linker -fdollars-in-identifiers -quiet')
MASPSX = ('--gnu-as-path mipsel-linux-gnu-as --aspsx-version=2.34 '
          '--run-assembler --expand-div -EL -Iinclude -Iinclude/psyq '
          '-Iinclude/decomp -O2 -G0 -march=r3000 -mtune=r3000 '
          '-no-pad-sections')

COMPILE_SH = """#!/bin/sh
# Invoked by decomp-permuter as: ./compile.sh input.c -o output.o
# The input is already preprocessed, so this is cc1 -> maspsx -> as.
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
cd "$ROOT"
"$ROOT/tools/bin/gcc-2.6.0/cc1" {cc1flags} -o "$TMP/x.s" "$INPUT"
"$ROOT/.venv/bin/python3" "$ROOT/tools/maspsx/maspsx.py" {maspsx} \\
    -o "$OUT" "$TMP/x.s" < /dev/null
"""

SETTINGS = 'func_name = "{sym}"\ncompiler_type = "gcc"\n'


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode:
        raise SystemExit('failed: %s\n%s' % (' '.join(cmd), r.stderr or r.stdout))
    return r


def setup(target, obj, sym):
    unit = obj.split('/')[-1]
    src = '%s/build/nm/JP1/src/%s.sjis.i' % (ROOT, obj)
    if not os.path.exists(src):
        raise SystemExit('no preprocessed unit at %s - run build/one.sh first' % src)
    asm = '%s/asm/JP1/%s/nonmatchings/%s/%s.s' % (ROOT, target, unit, sym)
    if not os.path.exists(asm):
        raise SystemExit('no asm for %s at %s' % (sym, asm))

    out = '%s/permuter/%s/%s' % (ROOT, target, sym)
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)

    shutil.copy(src, os.path.join(out, 'base.c'))

    sh = os.path.join(out, 'compile.sh')
    with open(sh, 'w', newline='\n') as f:
        f.write(COMPILE_SH.format(root=ROOT, cc1flags=CC1FLAGS, maspsx=MASPSX))
    os.chmod(sh, 0o755)

    with open(os.path.join(out, 'settings.toml'), 'w', newline='\n') as f:
        f.write(SETTINGS.format(sym=sym))

    # target.o: the original routine on its own, under the same symbol name.
    # The generated .s leans on macro.inc for glabel and friends, which the
    # ordinary build pulls in through the INCLUDE_ASM wrapper, so it has to be
    # included here too.
    tgt_s = os.path.join(out, 'target.s')
    with open(tgt_s, 'w', newline='\n') as f:
        f.write('.include "macro.inc"\n'
                '.section .text, "ax"\n'
                '.include "%s"\n' % asm)
    run(['mipsel-linux-gnu-as', '-EL', '-Iinclude', '-Iinclude/psyq',
         '-Iinclude/decomp', '-I', 'build/JP1', '-O2', '-G0', '-march=r3000',
         '-mtune=r3000', '-no-pad-sections',
         '-o', os.path.join(out, 'target.o'), tgt_s], cwd=ROOT)

    # the scratch has to build before the permuter can use it
    r = subprocess.run([sh, os.path.join(out, 'base.c'),
                        '-o', os.path.join(out, 'base.o')],
                       capture_output=True, text=True, cwd=out)
    print('%s/%s -> permuter/%s/%s' % (target, sym, target, sym))
    print('   compile.sh %s' % ('ok' if r.returncode == 0 else 'FAILED'))
    if r.returncode:
        print((r.stderr or r.stdout).strip()[:800])
        return 1
    return 0


if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    sys.exit(setup(sys.argv[1], sys.argv[2], sys.argv[3]))
