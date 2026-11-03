# Split-hand cleanup

`func_8007CBE4` restores the blackjack table after a split hand. Its 2,024 bytes match the JP1 routine exactly with the pinned original compiler.

At timer 0 it clears the bet, slides the remaining card objects out, restores the visible hand labels, and moves the result objects and optional markers. Timer 8 collapses all six label pieces. Timer 32 hides the optional markers and rebuilds their layout. From timer 33 it restores result-object positions, hides the result sprites, returns the table pieces, and advances to step 18.

The timer used for the six timer-8 animations is a snapshot. Later phase checks read the live timer again, so a callback can advance the next phase in the same invocation. Result 11 resets two objects; results 12 and 13 reset one object's position and Y rotation. Other result values leave those transforms intact.

The host fixture covers 381,024 combinations of timer boundaries, result values, option states, card counts, empty and populated card-object ranges, palette state, and callbacks that change the timer. It checks callback arguments/order, transform writes, bet reset and final state, with optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds. This is a controller test, not a native game boot or gameplay test.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/blackjack_split_cleanup.c \
  -o /tmp/persona-split-cleanup
/tmp/persona-split-cleanup
```

The unchanged complete JP1 build must also pass all 11 independent binary SHA-256 checks. No game assets or generated binaries are included in this contribution.
