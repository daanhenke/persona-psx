#ifndef PERSONA_COMMON_SPELL_H
#define PERSONA_COMMON_SPELL_H

/* Persona 1 (JP) - the spell reference data.
 *
 * Twenty bytes a record in main's rodata, indexed by the ids a Persona carries
 * in spell[]. Only the name is worked out; the rest is here for the stride. */
#include <types.h>

#define SPELL_NAME_CELLS 10

typedef struct {
    /* 0x00 */ u_char name[SPELL_NAME_CELLS];
                                  /* tile bytes for one row, added to whichever
                                     glyph bank the drawer is given */
    /* 0x0A */ u_char pad0A[10];
} SpellData;                      /* 20 bytes */

extern SpellData g_spell_data[];

#endif
