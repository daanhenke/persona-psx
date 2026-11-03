# Hard card and reel matches

Three saved hard cases now use matching C, adding 516 bytes of linked-exact code:

| Function | Bytes | Source |
|---|---:|---|
| `func_8008EAA4` | 248 | `casino/game/handevaluate.c` |
| `func_80082340` | 96 | `casino/game/pairaces.c` |
| `D_8008B37C` | 172 | `casino/game/reelmask.c` |

The hand evaluator retains all eight hand-analysis calls and their state writes. Two nested state checks returning zero, followed by the default return of one, recover the original register allocation. A combined Boolean return had four instruction differences.

The ace counter uses a signed array index over two cards. GCC 2.6.0 reduces this to the original pointer loop and hoists the unsigned division constant before the pointer copy. The former explicit pointer loop left three instruction differences.

The reel mask uses signed row and column indices, with the row pointer derived inside the outer loop. This preserves the nine signed-byte symbol reads, halfword mask accumulation, and changed-palette callback. The compiler generates the original pointer induction and prologue; manually advancing the row pointer left four instruction differences.

Validation: all three functions are 100% in the regenerated objdiff report. All eleven JP1 SHA-256 checks pass after the report and a normal matching rebuild. No inline assembly, padding, binary edits, or compiler-option changes were introduced. No native Linux runtime or emulator gameplay test is claimed.

Two original priority predicates also now match: `FieldTileOneWay` (156 bytes) and `CharCanUsePersona` (140 bytes). In both cases nested conditions with a literal success return and default failure return recover the original allocation. Their noclip, tile direction, Persona bond and character level checks are preserved. The combined five-function batch adds 812 bytes.

The follow-up resolves `func_8008B428` (120 bytes) and `func_8008AA64` (148 bytes). Computing each palette index as `bit + 6` lets GCC reduce the loop to two original counters. Using a short intermediate for the paired sprite offset recovers the multiply-by-25 register chain; the masked player index is 0–255, so its product is at most 6375 and fits. Both are 100% in objdiff and all eleven linked hashes pass after report generation and the normal rebuild.
