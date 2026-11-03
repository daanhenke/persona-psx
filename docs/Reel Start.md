# Reel initialization

`func_80083224` (372 bytes) restarts reel audio, clears 21 halfword timing fields, sets initial reel states, randomizes each reel position during the first 21 ticks, and advances the casino state after tick 21. The assembly uses a halfword clear; the initial decompiler draft incorrectly inferred a byte.

Isolated linked bytes match, objdiff reports 100%, and all eleven JP1 SHA-256 checks pass.
