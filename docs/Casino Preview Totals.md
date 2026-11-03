# Casino preview totals

`func_80093614` refreshes ten pairs of values across two sprite layouts.
It saves each current and previous pair's signed-byte total before changing
the previous-value buffer. Changed components select textures at value minus
one. A changed total below ten uses one digit; otherwise it selects decimal
tens and units and enables the extra sprite. Both layouts are uploaded after
the loop, even when no values changed.

Byte sums intentionally wrap through signed eight-bit values. The two
20-byte tables are represented together, avoiding negative array indices.
The layout uses the shared project type. The original caller passes
promoted integer arguments to `CasinoSpritesSetOn`; the explicitly named
ABI alias retains this without declaring the project's short-argument
function under a conflicting C prototype. All reachable sprite IDs
(373–409) and the count (one) fit that callee's short parameters. A native
port should connect this interface through a typed adapter.

1,048,576 native fixtures cover all byte pairs, four prior-value patterns,
and four callback modes. They check equal totals with different components,
signed wrapping, all texture slots and canaries, sprite flags, call order,
changes to subsequent rows, layout redirection, and pointer reloading between
uploads. The adapter calls the actual project sprite setter with narrowed
arguments. Normal and ASan/UBSan runs pass with sanitizer recovery disabled.
Function tests do not establish native game startup or gameplay.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude -Iinclude/psyq tests/native/casino_previewdigits.c -o /tmp/casino-previewdigits
/tmp/casino-previewdigits
```
