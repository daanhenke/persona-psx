/* Persona 1 (JP) - the row of a persona's type label.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                      DNG         ADV         S2D
 *   PersonaTypeLabel   0x800992D4  0x80098B0C  0x80089784
 *
 * A persona's type is a two-digit code, tens and units each 1 to 4, and the
 * data screens print it from a table of labels ten cells wide that follows
 * the arcana names. 11 to 14 are rows 1 to 4, 21 to 24 rows 5 to 8, and so
 * on to 44 at 16; anything else is row 0. Written as the cases falling into
 * one another, each adding one on the way down.
 */
#include <decomp/types.h>

short PersonaTypeLabel(short code)
{
    int n = 0;

    switch (code) {
    case 44:
        n++;
    case 43:
        n++;
    case 42:
        n++;
    case 41:
        n++;
    case 34:
        n++;
    case 33:
        n++;
    case 32:
        n++;
    case 31:
        n++;
    case 24:
        n++;
    case 23:
        n++;
    case 22:
        n++;
    case 21:
        n++;
    case 14:
        n++;
    case 13:
        n++;
    case 12:
        n++;
    case 11:
        n++;
    }
    return n;
}
