# S2D event flag selection

`func_8008BBD8` adjusts the selected event flag by one or five, clamps its
index to 0–512, and toggles the selected bit when the relevant input is set.
Input reads remain separate calls to the existing selector.

The selected signed halfword at `0x800A4D04` was previously represented as
offset eight within the broad `D_800A4CFC` data region. Giving this field its
own symbol reproduces direct load/store addressing. Separate upward and
downward locals preserve the original register allocation. The shared flag
bank retains its existing PS1 address at `0x801F29C8`.

Acceptance requires the original 240-byte routine to match in objdiff and
all eleven JP1 linked binary SHA-256 checks to pass. This does not add a
native input or save-data backend.
