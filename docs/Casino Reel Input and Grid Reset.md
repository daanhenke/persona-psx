# Casino reel input and grid reset

`func_80085CF8` (528 bytes) consumes four manual reel flags. For each idle reel it advances its signed-byte rotation modulo four, refreshes that reel through `func_800871BC`, and clears the input flag.

`func_8008BFC4` (424 bytes) resets grid selection state, disables two lamp bits, selects the active spot record, and clears four 3×3 grids plus nine 3×3 planes. Keeping the two zero-assignment chains separate reproduces the original compiler's address allocation and store order. The lamp word is declared as an array so the old compiler retains its shared address.

All three replacements reproduce their complete linked function bytes, score 100% in objdiff, and pass all eleven JP1 binary SHA-256 checks in the pinned build environment. These are PS1 matching implementations; they do not add a native renderer or replace the PS1 SDK.

`func_80092F14` (612 bytes) computes transform deltas, updates the saved rotation and position components, counts changed components, and starts a one-frame animation if needed. Its 40-byte initializer remains the original `D_80065D54` record. Scale differences are counted without updating the saved scale, matching the original behavior.
