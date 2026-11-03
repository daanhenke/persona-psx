# Scene image and reel result helpers

Three JP1 functions now use matching C, totaling 660 bytes:

- `func_8007F1D0` (224 bytes) runs four animation events in forward or
  reverse order for sides 0 and 1. Other side/event values are ignored.
- `func_80083B1C` (224 bytes) resolves the initial reel symbols, saves the
  result rank in both indexed tables, and advances the casino state.
- `func_8009C67C` (S2D, 212 bytes) resolves and reads a scene TIM, pumps the
  existing wait callback until the CD read completes, then queues its CLUT
  at (512, 243) and pixels at (640, 384). The original dimensions and
  eight-byte TIM-data offset are preserved.

All 11 JP1 executable/overlay SHA-256 checks pass with these source units
included. No emulator gameplay test was performed. The scene-image helper
still uses the original CD, TIM and VRAM interfaces.
