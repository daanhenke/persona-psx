# Casino object animation

`CasinoStepAnims` at CASINO `0x8006B8B0` advances every active object's rotation, translation and scale by its stored per-frame deltas. It clears the busy flag when an animation finishes, compacts the parallel transform/object/timer arrays, and draws the updated object. The loop decrements its index after removal so that the next animation is not skipped.

The original GCC 2.6 allocation is reproduced by copying the outer index into the removal index before loading the object, then using that index to obtain the delta transform. This lets the removal loop reuse the existing array offsets. The implementation uses ordinary C with the original compiler settings.

The 616-byte function and all eleven complete JP1 executable/overlay SHA-256 checks match. This verifies the PS1 build; native Linux gameplay is a separate task. The original compaction loop also copies the element at the active count, which is preserved for matching.
