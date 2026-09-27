#ifndef PERSONA_COMMON_CARD_H
#define PERSONA_COMMON_CARD_H

/* Persona 1 (JP) - the save file's header, shared by the card code the
 * resident and OPEN.EXE both compile and by the unit that defines it.
 */
#include <decomp/types.h>

/* The save file's header as the BIOS reads it: "SC", the icon's frame count,
   the file's size in blocks, then the Shift-JIS title the card manager shows
   - "Persona S-data N  LV N  N:NN:NN" - with its numbers patched in place,
   and the icon's CLUT and two frames. */
typedef struct {
    /* 0x00 */ char    magic[2];
    /* 0x02 */ u_char  type;
    /* 0x03 */ u_char  blocks;
    /* 0x04 */ u_char  title[64];
    /* 0x44 */ u_char  pad44[0x1C];
    /* 0x60 */ u_short clut[16];
    /* 0x80 */ u_char  icon[2][0x80];
} CardHeader;                            /* 0x180 bytes */

extern CardHeader g_card_header;

#endif
