# Map glyph projection

`func_80098F2C` draws byte-coded text terminated by 255. Each glyph uses
a 31-column texture atlas, eight-pixel horizontal spacing, and a four-vertex
7 by 12 scene rectangle. Projection writes the four packet coordinates;
the renderer supplies the atlas UVs and a color-dependent CLUT, then queues
each textured quad at priority zero. Even an empty string initializes the
packet and obtains its texture page.

The workspace has six vector slots followed by one textured quad, matching
the observed frame layout. Only four vector slots are active; the purpose
of the extra two slots is unknown. The projection's depth and flag outputs
share one local, as in the original. Coordinate and color local copies
preserve the original compiler's register allocation. SDK packed-coordinate
writes require a four-byte adapter on hosts where long is eight bytes.

76,801 native fixtures cover all 255 drawable codes and the terminator,
long strings, signed coordinate boundaries, color and texture-page wrapping,
call order, preserved packet fields, and callbacks that change the texture
base or text. They pass normally and under address/undefined-behavior
sanitizers with recovery disabled. The projection adapter writes packed
pairs of 16-bit coordinates without assuming a 32-bit host long.
These are function-level checks, not game boot or gameplay validation.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude -Iinclude/psyq tests/native/map_glyph.c -o /tmp/map-glyph
/tmp/map-glyph
```
