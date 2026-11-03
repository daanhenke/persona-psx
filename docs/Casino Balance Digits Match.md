# Casino balance digits match

`func_80090F58` renders four decimal digits from a selected unsigned balance, hides leading zeroes except the units digit, updates texture indices and refreshes the layout.

The 508-byte C implementation uses direct array reads, explicit sprite/layout offsets and separate branch-local call arguments to preserve the original compiler schedule. Direct Ghidra analysis independently confirmed the behavior; its full 508-byte function window was checked against the linked binary. Ghidra pseudocode alone did not preserve the original branch.

Validation: 100% objdiff match; all eleven JP1 binary SHA-256 checks pass and were independently rehashed. Local log: `hard-balancedigits-validation.log`. No emulator or native-port gameplay testing is claimed.
