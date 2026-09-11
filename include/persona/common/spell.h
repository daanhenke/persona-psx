#ifndef PERSONA_COMMON_SPELL_H
#define PERSONA_COMMON_SPELL_H

/* Persona 1 (JP) - the spell reference data.
 *
 * Twenty bytes a record in main's rodata, indexed by the ids a Persona carries
 * in spell[]. Only the name is worked out; the rest is here for the stride. */
#include <decomp/types.h>

#define SPELL_NAME_CELLS 10

typedef struct {
    /* 0x00 */ u_char name[SPELL_NAME_CELLS];
                                  /* tile bytes for one row, added to whichever
                                     glyph bank the drawer is given */
    /* 0x0A */ u_char kind;       /* what it does; the enemy AI masks with
                                     SPELL_KIND_MASK and treats four of the
                                     results as always worth casting */
    /* 0x0B */ u_char pad0B[2];
    /* 0x0D */ u_char target;     /* which set of targets it can reach; the AI
                                     hands this to BtlPickAiTarget          */
    /* 0x0E */ u_char cost;       /* SP, weighed against Char.sp            */
    /* 0x0F */ u_char pad0F;
    /* 0x10 */ u_char aim;        /* how the move is aimed, in the low nibble:
                                     1 and 8 at the one fighter the AI picks,
                                     2 and 4 at the front of that side and
                                     everything the pickable mask covers.
                                     BtlAimMove dispatches on it.           */
    /* 0x11 */ u_char pad11[3];
} SpellData;                      /* 20 bytes */

#define SPELL_KIND_MASK 0x3E

/* Below this id a spell costs SP and the caster has to be able to afford it;
   from here up the AI does not check. */
#define SPELL_FREE_FIRST 0x75

extern SpellData g_spell_data[];

#endif
