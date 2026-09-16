"""Try source variants of one unit and score them with odiff.

Each variant is a set of exact-text edits to the unit's .c file. The file is
built into build/nm with NON_MATCHING defined - the same object odiff scores -
once as it stands, then once per variant, and restored afterwards unless a
variant is kept.

  usage: sweep.py <unit> <Symbol[,Symbol...]> <variants-file>
                  [--keep NAME] [--args] [--all] [-nN] [--version VER]
         sweep.py btlp/placefallen BtlPlaceFallenStep variants.txt
         sweep.py btlp/debugailment BtlDebugMemberAilment,BtlDebugEnemyAilment v.txt --keep win

The variants file:

    ##### name-of-variant
    old text
    -----
    new text
    =====
    another old text
    ----- all
    new text for every occurrence

A variant opens with a `##### name` line and holds any number of edit blocks
split by `=====` lines. In a block, old text and new text are split by a
`-----` line, and each old text must occur exactly once in the file - unless
the separator reads `----- all`, which replaces every occurrence. An empty new
text deletes the old text together with the newline after it. Blocks are
matched whatever the file's line endings are.

Output: the unmodified source is scored first as `base`. Every variant then
prints one summary line per symbol; a symbol that scores better than its base
also prints its differing rows - with the operand rows once it is at 90% or
better, or always with --args. --all prints those rows for every variant, not
just the ones that scored better - one that scores worse can still be the one
whose residual is easier to close. -nN caps the rows as in odiff.py. With --keep
NAME that variant is left in the file at the end; otherwise the file is put
back exactly as it was, even on an error or ^C.

GAME_VERSION in the environment, or --version, picks the version (JP1 by
default). Run it with the project's own Python; it calls odiff.py with the same
interpreter.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ODIFF = os.path.join(ROOT, "scripts", "odiff.py")
SEP = re.compile(r"(?m)^-----( all)?[ \t]*\r?\n?")
BLOCK = re.compile(r"(?m)^=====[ \t]*\r?\n?")
VARIANT = re.compile(r"(?m)^##### ")


def parse_args(argv):
    opts = {"keep": None, "args": False, "all": False, "cap": 80,
            "version": os.environ.get("GAME_VERSION", "JP1")}
    rest = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--keep":
            opts["keep"] = argv[i + 1]
            i += 1
        elif a == "--version":
            opts["version"] = argv[i + 1]
            i += 1
        elif a == "--args":
            opts["args"] = True
        elif a == "--all":
            opts["all"] = True
        elif re.fullmatch(r"-n\d+", a):
            opts["cap"] = int(a[2:])
        elif a in ("-h", "--help"):
            sys.exit(__doc__)
        else:
            rest.append(a)
        i += 1
    if len(rest) != 3:
        sys.exit(__doc__)
    return rest, opts


def read_variants(path):
    raw = open(path, encoding="utf-8").read().replace("\r\n", "\n")
    chunks = VARIANT.split(raw)
    variants = []
    for chunk in chunks[1:]:
        name, _, body = chunk.partition("\n")
        variants.append((name.strip(), body))
    if not variants:
        sys.exit("%s holds no `##### name` variants" % path)
    return variants


def apply(text, body):
    """Apply one variant's blocks to text; return (new text, error)."""
    eol = "\r\n" if "\r\n" in text else "\n"
    for block in BLOCK.split(body):
        if not block.strip():
            continue
        m = SEP.search(block)
        if not m:
            return None, "a block without a ----- line"
        old = block[:m.start()].strip("\n")
        new = block[m.end():].strip("\n")
        old, new = old.replace("\n", eol), new.replace("\n", eol)
        every = bool(m.group(1))
        if new == "" and text.count(old + eol) == 1:
            old += eol
        n = text.count(old)
        if n == 0 or (n > 1 and not every):
            return None, "old text found %d times: %r" % (n, old[:80])
        text = text.replace(old, new)
    return text, None


def build(obj, version):
    try:
        os.remove(os.path.join(ROOT, obj))
    except FileNotFoundError:
        pass
    r = subprocess.run(["make", "GAME_VERSION=" + version, "NON_MATCHING=1",
                        "SKIP_ASM=1", "GEN_COMP_TU=1", "BUILD_BASE_DIR=build/nm",
                        obj], cwd=ROOT, capture_output=True, text=True,
                       errors="replace")
    if r.returncode:
        lines = [l for l in (r.stdout + r.stderr).splitlines()
                 if re.search(r":\d+:|undefined|No rule", l)]
        return "BUILD-FAILED " + " | ".join(lines[:3])
    return None


def odiff(unit, sym, cap, with_args):
    cmd = [sys.executable, ODIFF, unit, sym, "-n%d" % cap]
    if with_args:
        cmd.append("--args")
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    return (r.stdout + r.stderr).rstrip()


def percent(line):
    m = re.search(r":\s*([\d.]+)%", line)
    return float(m.group(1)) if m else -1.0


def main():
    (unit, syms, varfile), opts = parse_args(sys.argv[1:])
    syms = syms.split(",")
    src = os.path.join(ROOT, "src", unit + ".c")
    obj = "build/nm/%s/src/%s.c.o" % (opts["version"], unit)
    if not os.path.isfile(src):
        sys.exit("no such unit: " + src)
    variants = read_variants(varfile)
    if opts["keep"] and opts["keep"] not in [n for n, _ in variants]:
        sys.exit("--keep names no variant: " + opts["keep"])

    # latin-1 with no newline translation puts every byte back as it was
    orig = open(src, newline="", encoding="latin-1").read()
    final = orig
    try:
        err = build(obj, opts["version"])
        if err:
            sys.exit("== base: " + err)
        base = {}
        for sym in syms:
            head = odiff(unit, sym, opts["cap"], False).splitlines()
            base[sym] = percent(head[0] if head else "")
            print("== base: %s" % (head[0] if head else "no output"))

        for name, body in variants:
            text, err = apply(orig, body)
            if err:
                print("== %s: REP-FAIL %s" % (name, err))
                continue
            open(src, "w", newline="", encoding="latin-1").write(text)
            err = build(obj, opts["version"])
            if err:
                print("== %s: %s" % (name, err))
                continue
            if name == opts["keep"]:
                final = text
            for sym in syms:
                head = odiff(unit, sym, opts["cap"], False).splitlines()
                line = head[0] if head else "no output"
                score = percent(line)
                better = score > base[sym]
                print("== %s: %s%s"
                      % (name, "BETTER " if better else "", line))
                if better or opts["all"]:
                    rows = odiff(unit, sym, opts["cap"],
                                 opts["args"] or score >= 90).splitlines()[1:]
                    if rows:
                        print("\n".join(rows))
    finally:
        open(src, "w", newline="", encoding="latin-1").write(final)
        if final is not orig:
            print("kept", opts["keep"])
        # leave build/nm holding the object for the source as it now stands
        build(obj, opts["version"])


if __name__ == "__main__":
    main()
