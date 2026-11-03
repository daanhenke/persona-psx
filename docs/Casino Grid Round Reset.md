# Grid round reset

`func_8008A010` restores the grid for a new round, resets the prior players’ symbol and digit displays, scrolls the player list, turns each active symbol toward zero, and returns the controls when all nine cells and the rolling digit animation are finished.

The source preserves callback-visible timer and cell reloads, the original copies of the nine-byte symbol block and twelve-byte digit rows, the current cursor coordinates, signed modulo handling, and the separate total of cells that were already zero when visited. The original promoted-argument calls to CasinoScroll and the single-argument grow helper are retained; the grow call carries an unused second argument left by the original caller.

All eleven complete JP1 binaries reproduce their original SHA-256 checksums. The new function has 1,560 bytes and 100% object matching. The native fixture checks 128,000 cases covering timer and signed-counter boundaries, every byte-valued player extreme, invalid completion states, negative cell values, busy objects and callback mutations. It passes optimized and ASan/UBSan builds with recovery disabled and leak detection disabled. Native fixtures use aligned host-sized views of the pointer-bearing tables; the PS1 build verifies the original addresses and layout. These checks do not establish a playable native game.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude -Iinclude/psyq tests/native/casino_gridreset.c -o /tmp/casino-gridreset
/tmp/casino-gridreset
```

For sanitizer validation replace `-O2` with `-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all` and run with `ASAN_OPTIONS=detect_leaks=0`.
