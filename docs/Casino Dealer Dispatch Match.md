# Casino dealer dispatcher

`func_8007D3CC` dispatches five byte-state values to their existing handlers.
All other values return without calling a handler. Preserving the case order
0x12, 0x35, 0x14, 0x70, 0x71 reproduces the original compiler's 176-byte branch
tree without introducing a jump table.

Acceptance requires 100% objdiff and all eleven JP1 linked binary SHA-256
checks in the pinned original-compiler environment.
