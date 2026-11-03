# ADV decompression and box fade

`AdvUnpack` (564 bytes) and `AdvBoxFadeDown` (368 bytes) now replace their assembly fallbacks with C that reproduces the original JP1 bytes. The decompressor is used by ADV scene and event loading; the box fade draws and advances the screen-covering rectangle through the existing frame-render paths.

For decompression, arithmetic opcode scaling preserves the original widened value across back-reference decoding. Initializing each loop counter separately and spelling out counter, output-pointer and byte-count updates reproduces the original instruction scheduling. Overlapping back references remain byte-wise copies, and the original command-boundary size check and end marker remain intact. The function retains its original unbounded interface; it is not a general-purpose decoder for unchecked input.

The fade uses an aggregate box initializer. Its two identical terminal character-render branches are deliberately retained in C: GCC 2.6 allocates the endpoint and frame-mode values before folding the distinction. Combining them earlier changes the saved-register assignment. The branch introduces no additional runtime path in the matching binary. `AdvBoxFadeUp` remains behind its assembly fallback.

Both functions match after isolated linking and score 100% in objdiff. The complete original-compiler build passes all eleven JP1 binary SHA-256 checks, independently recomputed after the build. The decompressor was also exercised by local native opcode fixtures and sanitizer checks; those tests do not establish native gameplay or replace the PS1 binary checks.
