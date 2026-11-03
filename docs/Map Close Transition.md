# Map close transition

`func_80091D24` captures the field view, closes two shaded panels toward the centre, then brightens their inner edges before disabling display. Its 1,320 bytes match the original JP1 routine, including linked symbol addresses.

The workspace holds four textured screen strips, two gradient panels, two draw modes and a pair of ordering tables. The source uses typed arrays so host tests follow the host structure layout rather than assuming PS1 pointer sizes. Setting the ordering-table depth before deriving its tag pointer preserves the original compiler's setup order.

Each frame moves the panel edges by six pixels. They meet on frame 20, when the inner colour starts at zero; it then advances in steps of four through 252. The final frame is frame 83. The loop's intensity counter must be wide enough to reach 256. An eight-bit counter inferred from colour stores would never terminate.

The host fixture executes 72 cases / 5,976 frames, checking screen capture coordinates, texture strips, gradient coordinates and colours, ordering-table selection, primitive order and display shutdown. It covers either initial buffer, alternating or fixed buffers, and callbacks that change the clear colour. Both optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds pass 357,048 checks. Hardware calls are observed by test doubles; this does not claim native game boot or GPU rendering.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/map_close.c -o /tmp/persona-map-close
/tmp/persona-map-close
```

The pinned original compiler produces a 100% object match, and all 11 complete JP1 binary SHA-256 checks pass. No game data, tool binaries or generated output is included.
