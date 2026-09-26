/* Persona 1 (JP) - bars with an end cap.  DNG only.
 *   0x80089A4C TileMapWriteBarCapped   0x80089AA0 TileMapWriteBarCapped2
 *
 * TileMapWriteBar's head and body (box.c), with the last cell swapped for a
 * cap; the second draws the same shape from the next three glyphs. They sit
 * right after box.c, as ADV's and S2D's copies do (still asm there). Nothing
 * calls them, and no JP1 file holds either address.
 */
#include <decomp/types.h>

#define BAR_HEAD  0x19
#define BAR_BODY  0x1A
#define BAR_END   0x1B
#define BAR2_HEAD 0x20
#define BAR2_BODY 0x21
#define BAR2_END  0x22

void TileMapWriteBarCapped(short *dst, u_char width)
{
    *dst = BAR_HEAD;
    width--;
    while (width != 0) {
        dst++;
        if (width == 1) {
            *dst = BAR_END;
        } else {
            *dst = BAR_BODY;
        }
        width--;
    }
}

void TileMapWriteBarCapped2(short *dst, u_char width)
{
    *dst = BAR2_HEAD;
    width--;
    while (width != 0) {
        dst++;
        if (width == 1) {
            *dst = BAR2_END;
        } else {
            *dst = BAR2_BODY;
        }
        width--;
    }
}
