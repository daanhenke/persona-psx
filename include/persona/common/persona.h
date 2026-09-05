#ifndef PERSONA_COMMON_PERSONA_H
#define PERSONA_COMMON_PERSONA_H

/* Persona 1 (JP) - the Persona records.
 *
 * They sit in the save-game work area immediately after the five Char records,
 * so the address is the same in every overlay. Char.list[3] holds indices into
 * this array - the Personas a character carries - with 0xFF for empty. */
#include <types.h>

#define PERSONA_SPELLS 7

#define PERSONA_STATS 5

typedef struct {
    /* 0x00 */ u_char pad00[0x26];
    /* 0x26 */ u_char stat[PERSONA_STATS];
                                  /* the same five the character has, drawn as
                                     bars on the status screen and clamped at
                                     99 there */
    /* 0x2B */ u_char pad2B[2];
    /* 0x2D */ u_char spell[PERSONA_SPELLS];
                                  /* learned in order; 0 for a slot the
                                     Persona has not reached yet */
    /* 0x34 */ u_char pad34[0xC];
} Persona;                        /* 0x40 bytes */

#define g_personas ((Persona *)0x801F1DAC)

/* The reference data every Persona is built from, in main's rodata rather than
   the save-game area. The stock list shows an entry this way before it belongs
   to anybody. */
typedef struct {
    /* 0x00 */ u_char pad00[0x1C];
    /* 0x1C */ u_char name[10];   /* tile bytes, terminated by 0xFF */
    /* 0x26 */ u_char level;
    /* 0x27 */ u_char arcana;     /* 1-based, into a table of six-cell labels */
    /* 0x28 */ u_char stat[PERSONA_STATS];
    /* 0x2D */ u_char pad2D[0xB];
} PersonaData;                    /* 0x38 bytes */

extern PersonaData g_persona_data[];

#endif
