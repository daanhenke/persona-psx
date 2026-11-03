# Casino reward audio selector

`func_80093AD0` selects one of four sequence addresses using unsigned
thresholds 100, 1000 and 10000, then calls `CasinoPlaySeq` with the second
casino sequence slot and the main VAB. Its C implementation is 164 bytes.

The previous 172-byte symbol included a separate `jr ra; nop` stub at
`0x80093B74`. The symbol sizes now describe both regions separately; the
eight-byte stub remains assembly. No executable bytes are removed or changed.

Validation uses the pinned original compiler, objdiff, and all eleven JP1
linked binary SHA-256 checks. This is a matching PS1 implementation; it does
not introduce a native audio backend.
