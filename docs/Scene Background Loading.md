# Scene background loading

`AdvLoadEventBg` selects the scene's background archive from its story-flag variant, reads and unpacks it, then queues the enabled image members at their original VRAM coordinates. The first enabled image is deliberately queued twice, as in the original. Rooms of kinds 2, 3 and 4 also copy their respective room-data blocks before waiting for the GPU.

A shared unsigned-short temporary holds each flag and then its selected id. The image-presence local is an `int`, preserving the original zero extension. `ReadEventBg` returns the selected short id after the conditional read. Its inline boundary, together with the caller's initial sentinel guard, retains both original `0xFFFF` checks. Flattening those checks changes the generated code.

The original compiler reproduces all 1,180 function bytes. All eleven complete JP1 binaries pass their SHA-256 checks. No new native runtime fixture or native Linux gameplay claim accompanies this change.
