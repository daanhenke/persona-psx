"""Score one built routine against the image with objdiff-cli.

objdiff does the alignment and the relocation handling itself, so the number is
the same one `make report` publishes:

    BtlStageClose: 99.74%  418 same, 2 args, 1 missing, 0 extra, 0 replaced

- **same** - identical, relocations included.
- **args** - the same instruction with different operands. Nearly always the
  register allocator, and not worth reading until the three below are zero.
- **missing** - in the image, not in the build.  **extra** - the other way.
- **replaced** - a different instruction in the same place.

Only the three structural buckets print unless `--args` is given.

A routine that `objcmp.py` calls byte-exact should score 100%. If it does not,
the expected `.s` is naming something the C names differently - usually a
reference into the middle of a symbol the overlay does not own, which needs
`global_vram_start`/`global_vram_end` in the target's yaml and a `size:` on the
symbol in its sym file. `objcmp.py` remains the authority on byte-exactness.

Needs `expected/` and objdiff.json:

    tools/objdiff/objdiff_generate.py tools/objdiff/config-retail.yaml JP1 \\
        --build-base build/nm

rewrites them; `make objdiff-config` also works but wipes `expected/` and
rebuilds the whole tree.

  usage: odiff.py <unit> [Symbol] [--args] [-nN]
         odiff.py btlp/roundflow
         odiff.py btlp/roundflow BtlStageClose --args

  With no Symbol, every function in the unit is listed with its percentage,
  worst first - the quickest read on where a unit stands.
"""
import json
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KIND = {"DIFF_DELETE": "missing", "DIFF_INSERT": "extra",
        "DIFF_REPLACE": "replaced", "DIFF_ARG_MISMATCH": "args"}


def find_cli():
    """objdiff-cli, not tools/objdiff/objdiff - that one opens the GUI."""
    env = os.environ.get("OBJDIFF_CLI")
    if env:
        return env
    for path in (os.path.join(ROOT, "tools", "objdiff", "objdiff-cli"),
                 "/mnt/g/projects/persona-1-2-2/bin/objdiff-cli"):
        if os.path.isfile(path):
            return path
    found = shutil.which("objdiff-cli")
    if found:
        return found
    sys.exit("objdiff-cli not found; set OBJDIFF_CLI to its path")


def run(unit, sym):
    cmd = [find_cli(), "diff", "-p", ROOT, "-u", unit,
           "-o", "-", "--format", "json"]
    if sym:
        cmd.insert(-4, sym)
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stderr or r.stdout)
    return json.loads(r.stdout)


def funcs(d, side="left"):
    return [s for s in d[side]["symbols"]
            if s.get("kind") == "SYMBOL_FUNCTION"]


def text(row):
    return row.get("instruction", {}).get("formatted", "-")


def main():
    args = [a for a in sys.argv[1:] if a[:1] != "-"]
    flags = [a for a in sys.argv[1:] if a[:1] == "-"]
    if not args:
        sys.exit(__doc__)
    unit = args[0]
    sym = args[1] if len(args) > 1 else None
    cap = ([int(a[2:]) for a in flags if a[:2] == "-n" and a[2:].isdigit()]
           or [60])[0]

    if sym is None:
        d = run(unit, None)
        for f in sorted(funcs(d), key=lambda s: s.get("match_percent") or 0):
            print("  %-32s %6.2f%%" % (f["name"], f.get("match_percent") or 0))
        return

    d = run(unit, sym)
    left = [f for f in funcs(d) if f["name"] == sym]
    right = [f for f in funcs(d, "right") if f["name"] == sym]
    if not left or not right:
        sys.exit("%s not found in %s" % (sym, unit))
    pct = left[0].get("match_percent") or 0.0
    left, right = left[0]["instructions"], right[0]["instructions"]

    n = {}
    for row in left:
        k = row.get("diff_kind", "NONE")
        n[k] = n.get(k, 0) + 1
    print("%s: %.2f%%  %d same, %d args, %d missing, %d extra, %d replaced"
          % (sym, pct, n.get("NONE", 0), n.get("DIFF_ARG_MISMATCH", 0),
             n.get("DIFF_DELETE", 0), n.get("DIFF_INSERT", 0),
             n.get("DIFF_REPLACE", 0)))

    shown = 0
    for i, (a, b) in enumerate(zip(left, right)):
        k = a.get("diff_kind", "NONE")
        if k == "NONE" or (k == "DIFF_ARG_MISMATCH" and "--args" not in flags):
            continue
        if shown >= cap:
            print("  ... more not listed; raise the cap with -nN")
            break
        print("  %4d %-8s %-36s %s" % (i, KIND.get(k, k), text(a), text(b)))
        shown += 1


if __name__ == "__main__":
    main()
