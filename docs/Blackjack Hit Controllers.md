# Blackjack hit controllers

`func_80079C28` (1,252 bytes) handles an additional first-hand card. `func_8007BA60` (1,568 bytes) handles either split hand. Both replace assembly with matching C, including object animations, hiding the old card placeholders, total updates and result labels.

The timer is reloaded after callbacks. At tick 36 the split controller intentionally falls through from the first hand to the second hand’s hiding sequence. At tick 65 it snapshots the hand for dispatch, while later status callbacks reload the selected-hand byte. The first-hand card lookup uses the existing address alias one byte before the card array. No artificial register bindings, padding or instruction emission are used.

Validation: both functions report 100% object matching and the complete pinned JP1 build reproduces all eleven original SHA-256 checksums. The native fixture covers 1,259,520 cases across timer boundaries, valid hand sizes, object positions, invalid hand selectors, result values, callback mutations and byte truncation. It passes optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds with sanitizer recovery disabled. Leak detection is disabled. These are isolated control-flow tests, not a game boot or gameplay test.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude -Iinclude/psyq tests/native/blackjack_hit.c src/casino/game/bjhit.c src/casino/game/bjsplithit.c -o /tmp/blackjack-hit
/tmp/blackjack-hit
```

For sanitizer validation replace `-O2` with `-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all` and run with `ASAN_OPTIONS=detect_leaks=0`.
