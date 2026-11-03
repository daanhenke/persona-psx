# Movement and formation helpers

This JP1 batch adds six matching function instances, totaling 1,440 bytes:

- Four S2D facing-relative coordinate steps, `func_8008CB40`,
  `func_8008CC18`, `func_8008CCF8`, and `func_8008CDD8` (888 bytes).
  Each updates a 16-bit coordinate according to facing, then sets the
  associated state and direction bits. Coordinate wrap is preserved.
- Casino reward application, `func_8008D04C` (212 bytes). Reward kinds 0 and 1
  add 1,000,000 or 5,000 to the unsigned money total, then cap it at 99,999,999;
  kind 2 increments its byte counter. Other kinds leave state unchanged.
- S2D `FormationCellFree` (340 bytes). The DNG/ADV shared implementation
  already matched; enabling the same wrapper option with S2D's work-area
  bias reproduces the original S2D function too. The obsolete assembly unit
  boundary is removed, with no duplicate copy added.

All 11 JP1 executable/overlay SHA-256 checks pass. No emulator gameplay test
was performed. The original function names and out-of-range behavior are
preserved; the movement helpers do not validate or normalize facing values.

The S2D formation function is byte-exact after linking, but objdiff reports
99.76471% for its unlinked object: four operands compare symbolic HI16/LO16
references to D_8010B34C/D_8010B347 against literal addresses. There are no
instruction additions, deletions or replacements. All linked binary hashes
prove the enabled C matches. The published percentage retains objdiff's
conservative count, so this 340-byte function remains amber in the tree.
