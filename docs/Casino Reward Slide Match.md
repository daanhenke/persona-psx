# Casino reward slide

`func_8008E590` moves two reward objects left, applies eleven payout entries on alternating frames, and advances once movement and entry processing finish (352 bytes). Independent timer reads across calls preserve the original behavior.

The right object's position is named as a scalar field; treating it as an array element made the compiler retain an extra address register. The scalar declaration restores the original access sequence. The isolated original-compiler comparison has no differing instruction words; all eleven linked JP1 SHA-256 checks pass and were independently rehashed. The progress report scores 100%.
