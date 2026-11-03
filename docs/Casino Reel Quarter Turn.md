# Casino reel quarter turn

`func_800871BC` rotates one group of four peripheral reels. It restarts the
sequence sound, submits four 16-frame directional layout tweens, and snapshots
four byte fields plus a lookup-table pointer for each reel. A second loop
writes the rotated snapshot back and updates each second-bank layout's first
texture index from its signed byte offset into the selected lookup table.

The two passes matter: writing entries during the first pass would overwrite
values still needed by later reels. The rotation is read afresh for each loop
iteration. A tween callback runs before that iteration snapshots its fields,
so callback changes are visible without changing its already selected source
index. Layouts and table pointers are likewise read at their original sites.

The group must be 0–3 and the stored rotation 0–3. As in the original program,
the directional switch has no fallback for invalid rotation values. Callers
must maintain that invariant. Actual lookup pointers must allow the signed
byte index used by the renderer; fixture rows include a surrounding buffer to
exercise the signed conversion safely.

The recovered function is 736 bytes. Isolated compilation and linking at the
original addresses reproduce every instruction. Acceptance requires all eleven
complete JP1 binary hashes as well. The 65,536 native fixtures cover every group
and rotation, 256 byte patterns, shared texture storage, and audio/tween
callbacks changing rotations, sequence IDs, fields, lookup pointers and future
layout arguments. Optimized and ASan/UBSan builds pass with recovery disabled.
These fixtures use controlled callbacks and do not test the PS1 audio or GPU.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_reelquarter.c -o /tmp/reelquarter
/tmp/reelquarter
```
