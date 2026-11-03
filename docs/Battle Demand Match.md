# Battle demand dialogue match

`BtlTalkSceneDemand` handles the battle conversation's item, money, charm, and blood demands. Its 3,368-byte C implementation now keeps the chosen enemy's HP destination separate from the current-HP and maximum-HP values during the blood-demand update. The destination points directly to `healed->c.hp`; the capped total is written through that pointer. The existing enemy selection, requested HP amount, dialogue, and other state transitions remain intact.

This recovers the original compiler's pointer copy and register allocation without converting the HP total into a pointer. A previously saved experimental candidate that did the latter is invalid and is not used.

The regenerated objdiff report is 100% exact. All eleven JP1 executable/overlay SHA-256 checks pass after the report and final matching rebuild; the hashes were independently recomputed. No inline assembly, padding, compiler changes, reference-binary edits, or native/emulator gameplay claims are introduced.
