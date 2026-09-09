"""Shared half of the two Ghidra sync scripts beside this file.

The tree's symbol lists (configs/JP1/sym.<target>.txt) and the Ghidra project
hold the same names for the same addresses, and they drift the moment either
side is worked on alone. This works out what each side is missing.

A name is worth copying when one side has a real one and the other still has a
generated one - splat's func_/D_/jtbl_, Ghidra's FUN_/DAT_/LAB_. Where both
sides have real names and they disagree, nothing is written: that is a decision
about which name is better, not a sync, so it is only reported.

The overlays live in one Ghidra program as overlay blocks, so every target
except the four sub-EXEs is reachable from /Persona 1/JP PSX/SLPS_005.00.
"""
import os
import re

ROOT = r'G:\projects\persona-psx-decomp'

# target -> the Ghidra address space its code lives in
SPACES = {
    'main': 'ram', 'adv': 'OVL_ADV', 'btlp': 'OVL_BTLP', 'casino': 'OVL_CASINO',
    'dng': 'OVL_DNG', 'name': 'OVL_NAME', 's2d': 'OVL_S2D',
}
ALL = ['main', 'adv', 'btlp', 'casino', 'dng', 'name', 's2d']

SYM_LINE = re.compile(r'^(\s*)([A-Za-z_]\w*)(\s*=\s*)0x([0-9A-Fa-f]+)(\s*;)(.*)$')
GENERATED = re.compile(r'^(func|D|jtbl|FUN|DAT|LAB|SUB|UNK|OVL|PTR|ARRAY|s)_',
                       re.I)
# Ghidra's own labels for the innards of a switch, which are not names either
ARTEFACT = ('thunk_FUN_', 'switchdata', 'caseD_', 'switchD_', 'default_')


def generated(name):
    return bool(GENERATED.match(name)) or name.startswith(ARTEFACT)


def sym_path(target):
    return os.path.join(ROOT, 'configs', 'JP1', 'sym.%s.txt' % target)


def read_syms(target):
    """[(line index, name, address, is_func)] for one sym file, in file order."""
    out = []
    with open(sym_path(target)) as f:
        for i, line in enumerate(f):
            m = SYM_LINE.match(line.rstrip('\n'))
            if m:
                out.append((i, m.group(2), int(m.group(4), 16),
                            'type:func' in m.group(6)))
    return out


def ghidra_syms(program, target):
    """{address: (name, is_function)} for one target's space.

    Ghidra allows several labels at one address and only one of them is
    primary; that is the name it shows and the one a rename changes. Reading
    whichever came first made a renamed address look like it had never been
    touched, so the primary wins here and any others are ignored.
    """
    space = SPACES[target]
    out = {}
    for f in program.getFunctionManager().getFunctions(True):
        a = f.getEntryPoint()
        if a.getAddressSpace().getName() == space:
            out[a.getOffset()] = (f.getName(), True)
    st = program.getSymbolTable()
    for s in st.getAllSymbols(False):
        a = s.getAddress()
        if a is None or a.getAddressSpace().getName() != space:
            continue
        off = a.getOffset()
        if off in out:
            continue
        p = st.getPrimarySymbol(a)
        out[off] = ((p or s).getName(), False)
    return out


def compare(program, targets=None):
    """(to_ghidra, to_tree, clashes, missing) as (target, addr, ours, theirs).

    missing is what the tree has as a function and Ghidra has no function for.
    """
    targets = targets or ALL
    to_ghidra, to_tree, clashes, missing = [], [], [], []
    for t in targets:
        theirs = ghidra_syms(program, t)
        for _, name, addr, isfunc in read_syms(t):
            if addr not in theirs:
                if isfunc:
                    missing.append((t, addr, name, None))
                continue
            gname, gisfunc = theirs[addr]
            if isfunc and not gisfunc:
                missing.append((t, addr, name, gname))
            if generated(name) and not generated(gname):
                to_tree.append((t, addr, name, gname))
            elif not generated(name) and generated(gname):
                to_ghidra.append((t, addr, name, gname))
            elif name != gname and not generated(name):
                clashes.append((t, addr, name, gname))
    return to_ghidra, to_tree, clashes, missing


def summarise(targets, to_ghidra, to_tree, clashes, missing):
    print('%-8s %9s %8s %8s %8s' % ('target', '->ghidra', '->tree', 'clash',
                                    'no func'))
    for t in targets:
        n = lambda rows: sum(1 for r in rows if r[0] == t)
        print('%-8s %9d %8d %8d %8d'
              % (t, n(to_ghidra), n(to_tree), n(clashes), n(missing)))


def show(label, rows, limit=20):
    if not rows:
        return
    print('\n%s (%d):' % (label, len(rows)))
    for t, a, ours, theirs in rows[:limit]:
        print('  %-6s 0x%08X  %-30s %s' % (t, a, ours, theirs))
    if len(rows) > limit:
        print('  ... %d more' % (len(rows) - limit))
