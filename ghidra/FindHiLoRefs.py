# Find every reference to a set of absolute addresses by pairing `lui` with the
# following signed-16-bit displacement, across *every* memory block including
# the overlay spaces.
#
# Ghidra's own reference analysis resolves these only inside `ram`, and it drops
# the `lui at,%hi(X) / addu at,at,idx / lw r,%lo(X)(at)` array-indexing idiom
# entirely - so a global written through that idiom looks unreferenced. This
# walks the instruction stream and does the pairing by hand, propagating the hi
# half through the index `addu`.
#
# Edit TARGETS, then run against each program in turn.
TARGETS = {
    0x80055C1C: "D_80055C1C",
    0x80055C20: "D_80055C20",
}

listing = currentProgram.getListing()
fm = currentProgram.getFunctionManager()
hits = []

for blk in currentProgram.getMemory().getBlocks():
    if not blk.isExecute() or not blk.isInitialized():
        continue
    hi = {}          # register name -> high half currently held
    ii = listing.getInstructions(blk.getStart(), True)
    while ii.hasNext():
        ins = ii.next()
        if ins.getMinAddress().getOffset() > blk.getEnd().getOffset():
            break
        mn = ins.getMnemonicString()

        # Read side, two forms:
        #  (a) `lw rD, %lo(X)(base)`  - one operand holding (scalar, register)
        #  (b) `addiu rD, base, %lo(X)` - the address-materialisation form, where
        #      the register and the scalar are in *separate* operands. Missing
        #      (b) makes anything reached through a pointer look unreferenced.
        cands = []
        for i in range(ins.getNumOperands()):
            reg = sc = None
            for o in ins.getOpObjects(i):
                n = type(o).__name__          # fully qualified under PyGhidra
                if n.endswith("Register"):
                    reg = o.getName()
                elif n.endswith("Scalar"):
                    sc = o
            if reg is not None and sc is not None:
                cands.append((reg, sc))
        if not cands and mn in ("addiu", "addi", "ori", "addu", "add"):
            src = ins.getRegister(1)
            sc = ins.getScalar(2)
            if src is not None and sc is not None:
                cands.append((src.getName(), sc))
        for reg, sc in cands:
            if reg not in hi:
                continue
            addr = (hi[reg] + sc.getSignedValue()) & 0xFFFFFFFF
            if addr in TARGETS:
                f = fm.getFunctionContaining(ins.getMinAddress())
                hits.append((TARGETS[addr], str(ins.getMinAddress()),
                             f.getName() if f else "?", str(ins)))

        # Write side: track what the destination register now holds.
        r0 = ins.getRegister(0)
        if r0 is None:
            continue
        d = r0.getName()
        if mn == "lui":
            s = ins.getScalar(1)
            if s is not None:
                hi[d] = s.getUnsignedValue() << 16
            else:
                hi.pop(d, None)
        elif mn in ("addu", "add", "or"):
            # `addu at,at,idx` keeps the hi half; that is the indexing idiom.
            srcs = [r.getName() for r in
                    [ins.getRegister(1), ins.getRegister(2)] if r is not None]
            keep = [hi[s] for s in srcs if s in hi]
            if keep:
                hi[d] = keep[0]
            else:
                hi.pop(d, None)
        else:
            hi.pop(d, None)

print(currentProgram.getName())
for h in hits:
    print("  %-14s %-14s %-30s %s" % h)
print("  total %d" % len(hits))
