# Casino reel state dispatcher

`func_800828DC` now uses 280 bytes of matching JP1 C. It invokes the handler
selected by `g_casino_step`, then always runs `func_8008634C`, including for
unhandled states. States 0x71 and 0x73 share a handler. The case order and
103-entry jump-table region are preserved.

All 11 executable/overlay SHA-256 checks pass. This is still PS1 code;
no emulator or native Linux gameplay test was performed.
