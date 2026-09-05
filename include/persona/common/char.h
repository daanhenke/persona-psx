#ifndef PERSONA_COMMON_CHAR_H
#define PERSONA_COMMON_CHAR_H

/* Persona 1 (JP) - the playable characters.
 *
 * Five records in the save-game work area, so the address is the same in every
 * overlay. g_party holds a record index per party slot. */
#include <types.h>

typedef struct {
    /* 0x00 */ int     hp;      /* the status HUD draws the danger colour once
                                 hp drops to hp_max / 4, which is what tells
                                 the pair apart */
    /* 0x04 */ int     hp_max;
    /* 0x08 */ int     sp;
    /* 0x0C */ int     sp_max;  /* both maxima cap at 999 */
    /* 0x10 */ u_char  pad10[0x10];
    /* 0x20 */ u_short equip[7];  /* inventory entries; 0 for an empty slot */
    /* 0x2E */ u_char  pad2E[0x10];
    /* 0x3E */ u_char  key;       /* identifies the record; 0 while unused  */
    /* 0x3F */ u_char  pad3F[0xA];
    /* 0x49 */ u_char  status;    /* ailment code, 0 for none. Event scripts
                                     set and clear it; recovery items ask
                                     CharHasStatus for the one they cure. */
    /* 0x4A */ u_char  pad4A[2];
    /* 0x4C */ u_char  stat[5];
    /* 0x51 */ u_char  stat_base[5];
                                  /* The status screen draws stat_base as the
                                     bar and highlights stat - stat_base on top
                                     of it, and clamps both at 99. */
    /* 0x56 */ u_char  pad56[1];
    /* 0x57 */ u_char  entry;     /* which list slot is active, 0xFF for none;
                                     CharRecalcStats reads list[entry] */
    /* 0x58 */ u_char  list[3];   /* 0xFF marks an empty entry              */
    /* 0x5B */ u_char  pad5B[3];
    /* 0x5E */ u_char  blocked; /* while set the entry list reads as empty   */
    /* 0x5F */ u_char  pad5F[1];
} Char;                           /* 0x60 bytes */

#define g_chars ((Char *)0x801F1BCC)

#define CHAR_COUNT  5
#define CHAR_EQUIP  7
#define CHAR_LIST_N 3
#define CHAR_STATS  5

#endif
