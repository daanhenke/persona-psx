# Casino scaled step

`D_80091E30` updates the double at offset `0x58` by adding one tenth of
the double at offset `0xA0`. It converts the updated value to an integer,
multiplies that integer by eleven, and returns the low signed halfword.
The existing symbol name is retained pending broader semantic identification.

The implementation preserves the floating-point operation order and conversion
point. It reproduces the original 136-byte routine with the original compiler.
Acceptance requires 100% objdiff and all eleven JP1 linked SHA-256 checks.

The same validated batch also recovers `func_80081F80` (180 bytes). It returns
three when the first three card bytes all have remainder six modulo thirteen,
or 255 otherwise. Nested success checks preserve the original register
allocation; separate early failure returns produce different code.
