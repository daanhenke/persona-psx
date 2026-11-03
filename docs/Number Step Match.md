# Bounded number input match

`func_8008BCC8` (176 bytes) updates a number in the S2D overlay from directional input, uses a step of five when the modifier is active, and wraps at the supplied minimum and maximum. The two directional reads remain separate and retain their original order.

Computing the step from the first input call with a conditional expression places its initialization in the original branch delay slot. Updating the value parameter directly, rather than first copying it to another local, also preserves the original saved-register setup. Neither change requires inline assembly, padding, or compiler modifications.

The regenerated objdiff report is 100% exact. All eleven JP1 linked executable/overlay SHA-256 checks pass after the report and final matching rebuild, and the hashes were independently recomputed. Native runtime and emulator gameplay testing are outside this matching change.
