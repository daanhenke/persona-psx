# Apply names from config/p1-jp/rename.txt to the current program.
#
# Naming belongs in Ghidra, not in the C sources: tools/gen_names.py turns
# Ghidra's names into the linker symbol map, so renaming here makes every
# decompiled source and every future diff use the real name.
#
# rename.txt format (blank lines and // comments ignored):
#     <program> <space> <address> <name> [# comment]
# e.g.
#     SLPS_005.00  ram      800DC004  g_pad
#     SLPS_005.00  OVL_DNG  80075D88  SndSeqPlay
#     END.EXE      ram      80082B24  StrDecodeNextFrame
#
# The program column matters: every sub-EXE uses the "ram" space and they all
# load at 0x80080000, so the same address means different things in each.
import os

from ghidra.program.model.symbol import SourceType

PATH = r"G:\projects\persona-1-2-2\config\p1-jp\rename.txt"

fm = currentProgram.getFunctionManager()
st = currentProgram.getSymbolTable()
listing = currentProgram.getListing()
af = currentProgram.getAddressFactory()


def resolve(space_name, offset):
    for sp in af.getAllAddressSpaces():
        if sp.getName() == space_name:
            return sp.getAddress(offset)
    return None


applied = skipped = missing = 0
tid = currentProgram.startTransaction("apply names")
try:
    for raw in open(PATH):
        line = raw.split("//")[0].strip()
        if not line:
            continue
        comment = ""
        if "#" in line:
            line, comment = line.split("#", 1)
            comment = comment.strip()
        parts = line.split()
        if len(parts) < 4:
            continue
        program, space_name, addr_s, name = parts[0], parts[1], parts[2], parts[3]
        if program != currentProgram.getName():
            continue
        addr = resolve(space_name, int(addr_s, 16))
        if addr is None:
            print("  ?? unknown space %s" % space_name)
            missing += 1
            continue

        f = fm.getFunctionAt(addr)
        if f is not None:
            if f.getName() == name:
                skipped += 1
                continue
            old = f.getName()
            f.setName(name, SourceType.USER_DEFINED)
            if comment:
                listing.setComment(addr, 3, comment)
            print("  fn   %-10s %s  %-22s <- %s" % (space_name, addr_s, name, old))
            applied += 1
            continue

        sym = st.createLabel(addr, name, SourceType.USER_DEFINED)
        sym.setPrimary()
        if comment:
            listing.setComment(addr, 0, comment)
        print("  data %-10s %s  %s" % (space_name, addr_s, name))
        applied += 1
finally:
    currentProgram.endTransaction(tid, True)

print("applied=%d unchanged=%d unresolved=%d" % (applied, skipped, missing))
