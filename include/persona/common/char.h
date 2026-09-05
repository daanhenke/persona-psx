#ifndef PERSONA_COMMON_CHAR_H
#define PERSONA_COMMON_CHAR_H

/* Persona 1 (JP) - the playable characters.
 *
 * Five records in the save-game work area, so the address is the same in every
 * overlay. g_party holds a record index per party slot. */
#include <types.h>

typedef struct {
    /* 0x00 */ u_char  pad00[0x20];
    /* 0x20 */ u_short equip[7];  /* inventory entries; 0 for an empty slot */
    /* 0x2E */ u_char  pad2E[0x10];
    /* 0x3E */ u_char  key;       /* identifies the record; 0 while unused  */
    /* 0x3F */ u_char  pad3F[0xD];
    /* 0x4C */ u_char  stat[5];
    /* 0x51 */ u_char  stat_base[5];
                                  /* The status screen draws stat_base as the
                                     bar and highlights stat - stat_base on top
                                     of it, and clamps both at 99. */
    /* 0x56 */ u_char  pad56[2];
    /* 0x58 */ u_char  list[3];   /* 0xFF marks an empty entry              */
    /* 0x5B */ u_char  pad5B[5];
} Char;                           /* 0x60 bytes */

#define g_chars ((Char *)0x801F1BCC)

#define CHAR_COUNT  5
#define CHAR_EQUIP  7
#define CHAR_LIST_N 3
#define CHAR_STATS  5

#endif
