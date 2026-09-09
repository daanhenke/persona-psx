/* Persona 1 (JP) - the three string helpers for tile text.  BTLP only.
 *   0x80066394 BtlTileCopy  0x800663C4 BtlTileAppend  0x8006640C BtlTileLength
 *
 * Text on the battle screen is a run of glyph cell codes closed by GLYPH_END,
 * so these are strcpy, strcat and strlen over that terminator. Only the copy
 * is reached, and only by the append; the length is not called at all.
 *
 * The copy writes the terminator too, which is what lets the append find its
 * place by walking to the terminator and starting there.
 */
#include <decomp/types.h>
#include <persona/btlp/number.h>

void BtlTileCopy(u_char *dst, const u_char *src)
{
    int end;

    /* Through a variable: the original lifts the terminator out of the loop. */
    end = GLYPH_END;
loop:
    *dst = *src;
    dst++;
    if (*src == end) {
        return;
    }
    src++;
    goto loop;
}

void BtlTileAppend(u_char *dst, const u_char *src)
{
    while (*dst != GLYPH_END) {
        dst++;
    }
    BtlTileCopy(dst, src);
}

int BtlTileLength(const u_char *s)
{
    int n;

    n = 0;
    if (*s != GLYPH_END) {
        do {
            s++;
            n++;
        } while (*s != GLYPH_END);
    }
    return n;
}
