# Map tile initialization

`func_80094B08` clears transient bits 11–14 on all 40,000 map tiles,
retaining bit 15 and bits 0–10. The tile block starts at the unsigned
32-bit byte offset stored at map address `0x80122000`; rows contain
200 little-endian halfwords. It then clears the 20,000-byte packed map
and the 3,456-byte secondary state buffer.

For map zero, four byte-coordinate tables mark tiles with 0x3000,
0x5000, 0x2000 and 0x4000 respectively. An x coordinate of 255 terminates
each table. The first two loops retain their separate stores and cache
the tile block offset for each entry. The final callback runs for every
map ID. Valid inputs require in-range coordinates and a terminated table.

The packed-map pointer aliases offset 4 of `D_800B1D30`, as in the
existing packed-map helpers. The map ID is viewed as an unsigned
halfword to preserve the original load; zero detection is unchanged.
The library clear prototype matches the Psy-Q header.

Native validation uses an aligned map buffer in place of the fixed PS1
address. A test-only symbol alias places the packed-map pointer in a
naturally aligned host object. 960 fixtures check every tile and buffer
canary, empty and overlapping coordinate lists, high-bit map IDs, and
header/map-ID changes during callbacks. They pass normally and with
address/undefined-behavior sanitizers and recovery disabled. This is
function-level validation, not a game boot or gameplay test.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude tests/native/map_tileinit.c -o /tmp/map-tileinit
/tmp/map-tileinit
```
