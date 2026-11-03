# Palette and layout helpers

Three JP1 casino functions now use matching C, totaling 476 bytes:

- `func_80084444` (144 bytes) selects a result palette, queues its CLUT, and
  starts the associated palette animation. The indexed state byte keeps its
  original signed interpretation when selecting the sprite run.
- `func_8007EE78` (176 bytes) sends three layout pieces to the original tween
  routine, preserving the signed distance, duration, and width changes.
- `func_80089B68` (156 bytes) starts the selected layout's slide and scroll on
  its initial tick, then sets the next casino step and resets the timer.

All 11 JP1 executable/overlay SHA-256 checks pass with these source units
enabled. No emulator gameplay test was performed.
