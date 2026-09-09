"""Take into this tree the names Ghidra has and the symbol lists do not.

A PyGhidra script. Run it against /Persona 1/JP PSX/SLPS_005.00, which holds
every overlay as an overlay block.

From an inline snippet, which is also how to narrow it or rehearse it:

    import sys; sys.path.insert(0, r'G:\\projects\\persona-psx-decomp\\tools\\ghidra')
    import sync_from_ghidra
    sync_from_ghidra.main(currentProgram, targets=['btlp'], dry=True)

Only names go this way. Structs and signatures travel the other way, from the
C to Ghidra, because the C is where they are worked out - see sync_to_ghidra.py.

A name is taken when this tree still has a generated one (func_/D_/jtbl_) for
that address and Ghidra has a real one. Where both are real and they disagree
nothing is written; it says so instead.

After it writes, splat has to be re-run before the names reach the asm:

    touch configs/JP1/*.yaml && make generate
"""
import re
import sys

sys.path.insert(0, r'G:\projects\persona-psx-decomp\tools\ghidra')
import symsync

# splat writes these names straight into the asm and the C declares them, so a
# name that is not a C identifier cannot come across - Ghidra is happy with
# "2D_BG0_OBJ_1B4", the assembler is not
IDENT = re.compile(r'^[A-Za-z_]\w*$')


def main(program, targets=None, dry=False):
    targets = targets or symsync.ALL
    _, to_tree, clashes, _ = symsync.compare(program, targets)
    unusable = [r for r in to_tree if not IDENT.match(r[3])]
    to_tree = [r for r in to_tree if IDENT.match(r[3])]

    # splat needs one name per symbol file, and Ghidra is happy to call two
    # addresses the same thing - two copies of memclr, say. Anything that would
    # collide with a name the file already has, or with another incoming one,
    # stays behind.
    collide = []
    for t in targets:
        taken = {n for _, n, _, _ in symsync.read_syms(t)}
        rows, keep = [r for r in to_tree if r[0] == t], []
        for r in rows:
            if r[3] in taken:
                collide.append(r)
            else:
                taken.add(r[3])
                keep.append(r)
        to_tree = [r for r in to_tree if r[0] != t] + keep

    symsync.summarise(targets, [], to_tree, clashes, [])
    symsync.show('names to take from Ghidra', to_tree)
    symsync.show('not C identifiers, so left in Ghidra only', unusable)
    symsync.show('the name is already taken in the symbol file, so left alone',
                 collide)
    symsync.show('both named, disagreeing (left alone)', clashes)
    if dry:
        print('\ndry run: nothing written')
        return 0

    total = 0
    for t in targets:
        rows = {addr: theirs for tt, addr, _, theirs in to_tree if tt == t}
        if not rows:
            continue
        path = symsync.sym_path(t)
        with open(path) as f:
            lines = f.readlines()
        n = 0
        for i, name, addr, _ in symsync.read_syms(t):
            if addr in rows:
                lines[i] = lines[i].replace(name, rows[addr], 1)
                n += 1
        with open(path, 'w', newline='\n') as f:
            f.writelines(lines)
        print('  %s: %d renamed' % (t, n))
        total += n
    print('\nrenamed %d in the symbol lists' % total)
    if total:
        print('now: touch configs/JP1/*.yaml && make generate, then rebuild')
    return 0


try:
    main(currentProgram)          # noqa: F821 - Ghidra provides it
except NameError:
    pass
