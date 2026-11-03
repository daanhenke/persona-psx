# Field status ticks (JP1)

`func_80096514` at S2D address `0x80096514` updates field ailment damage and
SP recovery across the five character records. Its C implementation is
`src/s2d/game/fieldstatus.c` (712 bytes with the original compiler).

- Poison selects a two-tick damage cadence. Sickness selects a four-tick
  cadence, with poison also processed at the intermediate tick.
- Poison removes one HP; sickness removes one eighth of maximum HP. Both
  clamp current HP to at least one.
- The independent SP timer grants one SP every two ticks, capped at maximum
  SP. Unused character records are skipped.

The routine shares `scene_wait` and `scene_tick` with scene mode selection
and the alert effect. The scoped byte-sized status read and mixed-loop
increment order retain the original compiler's register allocation.

Validation passed the function's complete instruction match and all eleven
JP1 binary SHA-256 checks. This is PS1 matching work; it does not establish
native Linux gameplay.
