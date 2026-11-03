# S2D scene quad match

`func_8009C478` draws the loaded scene-image textured quad using the current map scale and restores the previous projection offsets afterward.

The 516-byte C implementation uses the SDK MATRIX, SVECTOR, VECTOR and POLY_FT4 types. Chained assignments preserve the original order of texture-coordinate and transform stores. The loop counter also supplies the projection result scratch space, matching the original stack layout.

Validation: 100% objdiff match; all eleven JP1 binary SHA-256 checks pass and were independently rehashed. Local log: `hard-scenequad-validation.log`. No emulator or native-port gameplay testing is claimed.
