# Export Ghidra's code/data map so splat can be driven from real analysis
# instead of spimdisasm's heuristics. One JSON per program; overlay blocks are
# reported as separate address spaces.
#
# funcs entries are [entry, min, max] per function (NOT merged) so exact
# per-function sizes survive; callers merge them only to compute data gaps.
import json, os

OUT_DIR = r"G:\projects\persona-1-2-2\config\p1-jp\ghidra"

fm = currentProgram.getFunctionManager()
mem = currentProgram.getMemory()
listing = currentProgram.getListing()

out = {"program": currentProgram.getName(), "spaces": {}}

for blk in mem.getBlocks():
    sp = blk.getStart().getAddressSpace().getName()
    d = out["spaces"].setdefault(sp, {"blocks": [], "funcs": [], "data": [],
                                      "names": {}, "sym_ranges": []})
    entry = {
        "name": blk.getName(),
        "start": blk.getStart().getOffset(),
        "end": blk.getEnd().getOffset(),
        "exec": bool(blk.isExecute()),
    }
    # Most globals live in uninitialized RAM (the BSS / work area), so symbol
    # lookups must cover every block - but the code/data split only makes sense
    # for blocks that actually have bytes.
    d["sym_ranges"].append([entry["start"], entry["end"]])
    if blk.isInitialized():
        d["blocks"].append(entry)

nfunc = nsplit = 0
for f in fm.getFunctions(True):
    ep = f.getEntryPoint()
    sp = ep.getAddressSpace().getName()
    d = out["spaces"].get(sp)
    if d is None:
        continue
    nfunc += 1
    body = f.getBody()
    ranges = list(body.getAddressRanges())
    if len(ranges) > 1:
        nsplit += 1
    # Emit each contiguous piece, tagged with the owning entry point.
    for r in ranges:
        d["funcs"].append([ep.getOffset(),
                           r.getMinAddress().getOffset(),
                           r.getMaxAddress().getOffset()])
    # Ghidra's PsyQ signature analyzer names SDK functions (PRNT_OBJ_594 etc.).
    # A default FUN_/thunk name means it is game code worth decompiling.
    d["names"]["%08X" % ep.getOffset()] = f.getName()
    # ...but a name we applied ourselves also lands in `names`, and that is game
    # code, not SDK. The symbol's source type is what separates the two: ours
    # are USER_DEFINED, the analyzer's are not. Consumers that want "is this
    # SDK" must ask this, not merely "does it have a name".
    sym = f.getSymbol()
    if sym is not None and str(sym.getSource()) == "USER_DEFINED":
        d.setdefault("user_named", []).append("%08X" % ep.getOffset())

ndata = 0
di = listing.getDefinedData(True)
while di.hasNext():
    dat = di.next()
    a = dat.getMinAddress()
    sp = a.getAddressSpace().getName()
    d = out["spaces"].get(sp)
    if d is None:
        continue
    ndata += 1
    d["data"].append([a.getOffset(), dat.getMaxAddress().getOffset()])

for sp, d in out["spaces"].items():
    d["funcs"].sort(key=lambda x: x[1])
    d["data"].sort()

# Non-default labels: SDK data, plus anything we have named ourselves. These let
# the C use real global names instead of D_<address> externs.
nsym = 0
DEFAULT_PREFIXES = ("DAT_", "LAB_", "FUN_", "SUB_", "UNK_", "EXT_", "OFF_",
                    "PTR_", "s_", "u_", "caseD_", "switchD_", "thunk_")
si = currentProgram.getSymbolTable().getAllSymbols(True)
while si.hasNext():
    sym = si.next()
    if not sym.isPrimary():
        continue
    a = sym.getAddress()
    if a is None or not a.isMemoryAddress():
        continue
    sp = a.getAddressSpace().getName()
    d = out["spaces"].get(sp)
    if d is None:
        continue
    nm = sym.getName()
    if nm.startswith(DEFAULT_PREFIXES):
        continue
    d.setdefault("symbols", {})["%08X" % a.getOffset()] = nm
    # As with functions: a USER_DEFINED label is one we applied, so it names a
    # real object. The PsyQ signature import also drops labels in the middle of
    # SDK code Ghidra never turned into functions (MTX_00_OBJ_5A0 and friends),
    # and those must not be mistaken for the start of a data object.
    if str(sym.getSource()) == "USER_DEFINED":
        d.setdefault("user_symbols", []).append("%08X" % a.getOffset())
    nsym += 1

try:
    os.makedirs(OUT_DIR)
except OSError:
    pass
name = currentProgram.getName().replace(".", "_")
path = os.path.join(OUT_DIR, name + ".json")
with open(path, "w") as fh:
    json.dump(out, fh)

print("%s -> %s" % (currentProgram.getName(), path))
print("  spaces=%d funcs=%d (noncontiguous=%d) data=%d symbols=%d" % (
    len(out["spaces"]), nfunc, nsplit, ndata, nsym))
for sp in sorted(out["spaces"]):
    d = out["spaces"][sp]
    print("    %-14s pieces=%d data=%d" % (sp, len(d["funcs"]), len(d["data"])))
