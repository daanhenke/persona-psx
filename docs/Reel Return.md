# Reel return

`func_80083F8C` restores palettes for eight consecutive reel sprites and six sprites spaced eight slots apart, slides the layout and object by 320 pixels over 32 frames, clears the bet, sets the timer to -1 and selects state 0x12.

The local palette initializer owns 14 bytes at CASINO offset `0xE38`. Two alignment bytes and the following four raw padding bytes preserve the next table at `0xE4C`. The two loops reuse their counter, reproducing the original induction variables and saved registers.

The original copies seven palette entries to the stack but reads eight. Its eighth value comes from uninitialized stack storage. This matching source deliberately retains that out-of-bounds read; it is not suitable for a native port until that behavior is investigated and resolved. No native runtime test is claimed.

Acceptance requires the original compiler's 316-byte function match and all eleven complete JP1 SHA-256 binary checks.
