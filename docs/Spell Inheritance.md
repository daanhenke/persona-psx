# Spell inheritance

`FuseInheritSpell` scans the source Persona's arcana-group classes in table
order. Each class names a start and length in the shared spell list. It returns
the first spell found among the source's first five spell slots that the target
Persona may inherit, together with its rank in the concatenated class runs.
Failure leaves both outputs at 255 and returns zero.

The original routine tests the sentinel in the first class row, even when it
reads runs from another group's row. Empty runs add no rank. These details and
the five-slot limit are preserved. The run start and length use separate `int`
locals; the loop indices remain `short`. This produces the original 488 bytes
without assembly fallbacks.

The native fixture uses the real `PersonaDataSpellIndex` and `FuseSpellAllowed`
helpers. Its 1,803 cases cover three class orders, all thirty fixture spells,
every searchable source slot, existing-spell and type restrictions, the sixth
slot, early sentinels, empty runs, and first-eligible ordering. It runs normally
and under address/undefined-behavior sanitizers. Compile with `SKIP_ASM`,
`WORK_BIAS=0`, GNU89 inline semantics, and function/data sections plus linker
garbage collection to omit the unit's unrelated PS1-address routines.

Acceptance also requires all eleven complete JP1 binary SHA-256 checks. These
fixtures do not establish emulator gameplay or native Linux game startup.
