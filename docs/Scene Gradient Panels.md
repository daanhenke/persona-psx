# Scene gradient panels

`func_80099AD8` renders three transformed gradient quadrilaterals from the
40-byte records also updated by `func_80099A00`. It copies the translation
vector once, sets black upper corners and blue-purple lower corners, then
builds each panel's vertices, applies rotation/translation/scale, projects the
four corners and submits the polygon to the supplied ordering table.

Flag bits 1–2 select the origin: top left (0), centered (1), or top right (2).
Mode 3 retains the previous panel's offsets; the first panel must use 0–2.
The original code leaves those offsets undefined if its first panel uses 3.
The C implementation preserves that precondition rather than inventing a
new fallback. Signed odd dimensions use arithmetic right-shift rounding.

The native fixture exercises all three valid first modes, all four later
modes, signed dimension boundaries, priority truncation, translation-copy
lifetime, callback changes and polygon reuse across the three submissions.
Its projection stub writes the original packed pair of 16-bit coordinates;
it does not emulate the PlayStation geometry engine. These fixtures are not
an emulator playthrough or proof of a working native game renderer.

Acceptance requires all eleven complete JP1 binary hashes and an exact
648-byte match, in addition to the 70,560 native and sanitizer cases.

## Message-window backdrop

`func_800937FC` clips/submits the message window, updates its horizontal
position, and optionally draws its transformed backdrop. It selects a solid
blue or a black-to-blue gradient, computes width from the message length,
and restores the previous projection origin after sorting the polygon.
The heading-dependent +240 offset reaches the geometry helper as a full
integer; truncating it to a short would change boundary behavior.

`D_800B93B8`, `D_800B93BC`, and `D_800B93A4` alias the window's `x`, `x0`,
and vertical scale fields. The native harness binds these labels to the
corresponding host-layout fields; treating them as independent globals would
hide important behavior. Its test-only assembler aliases do not enter the
PS1 source or binary. The rendering source itself is ordinary C.

30,240 fixtures cover flag/heading combinations, position and width boundaries,
solid/gradient colors, projection restoration, callback mutations, buffer
selection and these aliases. Projection is stubbed as packed 16-bit pairs.
The 740-byte routine still requires the complete eleven-binary acceptance gate.
