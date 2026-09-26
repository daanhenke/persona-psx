#ifndef PERSONA_COMMON_CEL_H
#define PERSONA_COMMON_CEL_H

/* Persona 1 (JP) - the cel lists the menus and the cinema draw from.
 *
 * A list is a head followed by its cels, eight bytes each. The config
 * list's tactics page sets the height of two of the cinema's lists.
 */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ u_char  count;
    /* 0x01 */ u_char  unk1;
    /* 0x02 */ u_short attr;
    /* 0x04 */ u_short w, h;
} CelHead;

extern CelHead g_cinema_cels0, g_cinema_cels1, g_cinema_cels3,
               g_cinema_cels4, g_cinema_cels5, g_cinema_cels6;

#endif
