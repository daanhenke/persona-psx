"""Push what this tree knows into Ghidra: names, function boundaries, the
struct definitions from include/persona, and the signatures of every routine
that has been decompiled.

A PyGhidra script. Run it against /Persona 1/JP PSX/SLPS_005.00, which holds
every overlay as an overlay block.

From an inline snippet, which is also how to narrow it or rehearse it:

    import sys; sys.path.insert(0, r'G:\\projects\\persona-psx-decomp\\tools\\ghidra')
    import sync_to_ghidra
    sync_to_ghidra.main(currentProgram, targets=['btlp'], dry=True)
    sync_to_ghidra.main(currentProgram, do_types=False)   # names only

Names are only given where Ghidra still has a generated one. Where both sides
have a real name and they disagree, what happens is the caller's decision, made
per run rather than recorded anywhere:

    clashes='skip'   report them and change nothing            (the default)
    clashes='ours'   this tree's name wins

Nothing goes the other way here - to take a name from Ghidra, use
sync_from_ghidra.py.

Signatures come from the C definitions in src/, so they carry the parameter
names and types the decompilation actually settled on - which is the point of
pushing them.
"""
import os
import re
import sys

sys.path.insert(0, r'G:\projects\persona-psx-decomp\tools\ghidra')
import symsync

ROOT = symsync.ROOT

# headers to hand Ghidra's C parser, most basic first
TYPE_HEADERS = ['include/decomp/types.h']
TYPE_DIRS = ['include/persona']

# a definition at column 0: "u_short BtlTalkScoreLine(short said, short weight)".
# The separator between the type and the name has to be non-empty, or the type
# part gobbles the name and hands back one letter.
DEF_START = re.compile(r'^([A-Za-z_]\w*(?:\s+[A-Za-z_]\w*)*?)(\s*\*+\s*|\s+)'
                       r'([A-Za-z_]\w*)\s*\(')
SKIP = ('if', 'while', 'for', 'switch', 'return', 'else', 'do', 'INCLUDE_ASM',
        'INCLUDE_RODATA', 'typedef', 'struct', 'union', 'enum', 'sizeof')


def c_signatures():
    """{function name: "ret name(params)"} from every definition in src/."""
    out = {}
    for dirpath, _, names in os.walk(os.path.join(ROOT, 'src')):
        for n in names:
            if not n.endswith('.c'):
                continue
            path = os.path.join(dirpath, n)
            with open(path, encoding='utf-8', errors='replace') as f:
                lines = f.read().split('\n')
            i = 0
            while i < len(lines):
                m = DEF_START.match(lines[i])
                if (m and m.group(3) not in SKIP
                        and m.group(1).split()[0] not in SKIP
                        and not lines[i].startswith('extern')):
                    sig = lines[i]
                    j = i
                    while sig.count('(') > sig.count(')') and j + 1 < len(lines):
                        j += 1
                        sig += ' ' + lines[j].strip()
                    nxt = lines[j + 1].strip() if j + 1 < len(lines) else ''
                    if nxt == '{' and sig.rstrip().endswith(')'):
                        # `static` is about linkage, and Ghidra's parser has no
                        # use for it in a signature
                        text = ' '.join(sig.split())
                        if text.startswith('static '):
                            text = text[7:]
                        out[m.group(3)] = text
                    i = j
                i += 1
    return out


def push_types(program, monitor):
    """Parse include/persona and decomp/types.h into the program's types."""
    from ghidra.app.util.cparser.C import CParserUtils
    from ghidra.program.model.data import DataTypeManager
    files = [os.path.join(ROOT, h) for h in TYPE_HEADERS]
    for d in TYPE_DIRS:
        for dirpath, _, names in os.walk(os.path.join(ROOT, d)):
            files += [os.path.join(dirpath, n) for n in sorted(names)
                      if n.endswith('.h')]
    incs = [os.path.join(ROOT, 'include'),
            os.path.join(ROOT, 'include', 'psyq'),
            os.path.join(ROOT, 'include', 'decomp')]
    args = ['-D__attribute__(x)=', '-D__extension__=']
    try:
        msg = CParserUtils.parseHeaderFiles(
            None, files, incs, args, program.getDataTypeManager(), monitor)
        # newer Ghidra hands back a results object rather than a string
        bad = [l for l in str(msg).split('\n') if 'rror' in l]
        print('parsed %d headers; %d complaints' % (len(files), len(bad)))
        for l in bad[:10]:
            print('   ' + l.strip())
    except Exception as e:
        print('header parse unavailable (%s); types left alone' % e)


def push_signatures(program, targets, sigs, dry):
    """Apply the C definition's signature to each function Ghidra has."""
    from ghidra.app.cmd.function import ApplyFunctionSignatureCmd
    from ghidra.app.util.cparser.C import CParserUtils
    from ghidra.program.model.symbol import SourceType
    fm = program.getFunctionManager()
    done = skipped = failed = 0
    for t in targets:
        space = symsync.SPACES[t]
        for f in fm.getFunctions(True):
            a = f.getEntryPoint()
            if a.getAddressSpace().getName() != space:
                continue
            sig = sigs.get(f.getName())
            if not sig:
                skipped += 1
                continue
            if dry:
                done += 1
                continue
            try:
                # the three-argument form has two overloads and a null first
                # argument cannot pick between them; the four-argument one is
                # unambiguous
                d = CParserUtils.parseSignature(None, program, sig + ';', False)
                if d is None:
                    failed += 1
                    continue
                ApplyFunctionSignatureCmd(a, d, SourceType.USER_DEFINED) \
                    .applyTo(program)
                done += 1
            except Exception as e:
                failed += 1
                if failed <= 8:
                    print('   %s: %s' % (f.getName(), str(e)[:110]))
    print('signatures: %d applied, %d with no C definition, %d failed'
          % (done, skipped, failed))


def main(program, targets=None, dry=False, do_types=True, do_sigs=True,
         clashes='skip'):
    if clashes not in ('skip', 'ours'):
        raise ValueError("clashes must be 'skip' or 'ours'")
    targets = targets or symsync.ALL
    to_ghidra, _, disagree, missing = symsync.compare(program, targets)
    contested = disagree if clashes == 'ours' else []
    symsync.summarise(targets, to_ghidra, [], disagree, missing)
    symsync.show('names to give Ghidra', to_ghidra)
    symsync.show('functions Ghidra does not have', missing)
    symsync.show('both named, disagreeing - %s'
                 % ('this tree wins' if contested else
                    "left alone; pass clashes='ours' to overrule Ghidra"),
                 disagree)
    if dry:
        print('\ndry run: nothing written')
        if do_sigs:
            push_signatures(program, targets, c_signatures(), True)
        return 0

    from ghidra.program.model.symbol import SourceType
    from ghidra.program.model.address import AddressSet
    from ghidra.program.flatapi import FlatProgramAPI
    from ghidra.util.task import ConsoleTaskMonitor
    monitor = ConsoleTaskMonitor()
    # createFunction and friends are script builtins; an imported module has to
    # ask for them
    flat = FlatProgramAPI(program, monitor)
    fm = program.getFunctionManager()
    st = program.getSymbolTable()
    spaces = {s.getName(): s
              for s in program.getAddressFactory().getAllAddressSpaces()}

    tx = program.startTransaction('sync from the decomp tree')
    made = renamed = failed = 0
    try:
        for t, off, name, _ in missing:
            a = spaces[symsync.SPACES[t]].getAddress(off)
            try:
                if fm.getFunctionAt(a) is None:
                    covering = fm.getFunctionContaining(a)
                    if covering is not None:
                        covering.setBody(AddressSet(covering.getEntryPoint(),
                                                    a.subtract(1)))
                    if flat.createFunction(a, None) is not None:
                        made += 1
            except Exception as e:
                failed += 1
                print('  create 0x%08X %s: %s' % (off, name, str(e)[:110]))

        for t, off, name, _ in to_ghidra + contested:
            a = spaces[symsync.SPACES[t]].getAddress(off)
            try:
                f = fm.getFunctionAt(a)
                if f is not None:
                    f.setName(name, SourceType.USER_DEFINED)
                else:
                    s = st.getPrimarySymbol(a)
                    if s is None:
                        st.createLabel(a, name, SourceType.USER_DEFINED)
                    else:
                        s.setName(name, SourceType.USER_DEFINED)
                renamed += 1
            except Exception as e:
                failed += 1
                print('  rename 0x%08X %s: %s' % (off, name, str(e)[:110]))

        if do_types:
            push_types(program, monitor)
        if do_sigs:
            push_signatures(program, targets, c_signatures(), False)
    finally:
        program.endTransaction(tx, True)
    print('\ncreated %d functions, renamed %d, %d failed' % (made, renamed, failed))
    return failed


try:
    main(currentProgram)          # noqa: F821 - Ghidra provides it
except NameError:
    pass
