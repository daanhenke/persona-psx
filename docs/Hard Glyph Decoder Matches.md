# Matching text-code helpers

Two previously deferred text-code helpers now reproduce the original code:

- `func_8008C6E4` (64 bytes) preserves the original decision tree, including its explicit 0x5181 check and signed range comparison after conversion to a 16-bit code. An integer argument masked with 0xFFFF keeps GCC's signed comparison; an unsigned-short formal generated `sltiu` instead of `slti`.
- `func_8008C724` (248 bytes) maps encoded ranges to glyph rows. Its 0xED82 arm assigns 0x1004E to an integer local before storing the low halfword. A direct assignment to the halfword destination removed the original high-word instruction. The remaining range arithmetic and switch arms match without changes.

Both implementations passed all eleven linked JP1 binary SHA-256 checks using the pinned offline environment and original GCC 2.6.0 toolchain. Exact matching remains separate from native Linux runtime work.
