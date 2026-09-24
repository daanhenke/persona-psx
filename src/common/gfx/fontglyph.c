/* Persona 1 (JP) - one font glyph into VRAM.
 *   DNG 0x800770EC   ADV 0x80067924   S2D 0x80067114
 *
 * The message window draws text one glyph at a time: the 1bpp bitmap is
 * expanded into a 4bpp staging cell and queued as a 16x16 upload. The glyph
 * goes down twice - first as a shadow in colour 1, a row lower and two pixels
 * right, then itself in colour 2 on top of it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/font.h>
#include <persona/common/imageanim.h>

/* Byte j of row i of the glyph. */
#define FONT(i, j) src[i][j]

/* 82.81%: gcc expands each store's address before its value, so the buffer
   row is hoisted ahead of the font row; the original has them the other way
   round. A temporary for the value fixes the order but narrows the sum, and
   the adds turn into ors. */
#ifdef NON_MATCHING
void FontUploadGlyph(short x, short y, u_short glyph)
{
    u_char (*src)[2];
    u_char (*buf)[8];
    u_char  i;
    u_char  j;

    src = (u_char (*)[2])(g_font_bits + glyph * GLYPH_BYTES);
    buf = (u_char (*)[8])g_glyph_buf;
    for (i = 0; i < 8; i++) {
        buf[0][i] = 0;
    }

    /* The shadow: written outright, one row down and one byte right. */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 2; j++) {
            buf[i][j * 4 + 9] = ((FONT(i, j) & 0x40) >> 2) + (FONT(i, j) >> 7);
            buf[i][j * 4 + 10] = (FONT(i, j) & 0x10) + ((FONT(i, j) & 0x20) >> 5);
            buf[i][j * 4 + 11] = ((FONT(i, j) & 0x4) << 2) + ((FONT(i, j) & 0x8) >> 3);
            buf[i][j * 4 + 12] = ((FONT(i, j) & 0x2) >> 1) + ((FONT(i, j) & 0x1) << 4);
        }
    }

    /* The glyph itself, over it. */
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 2; j++) {
            buf[i][j * 4 + 0] |= ((FONT(i, j) & 0x40) >> 1) + ((FONT(i, j) & 0x80) >> 6);
            buf[i][j * 4 + 1] |= ((FONT(i, j) & 0x10) << 1) + ((FONT(i, j) & 0x20) >> 4);
            buf[i][j * 4 + 2] |= ((FONT(i, j) & 0x4) << 3) + ((FONT(i, j) & 0x8) >> 2);
            buf[i][j * 4 + 3] |= (FONT(i, j) & 0x2) + ((FONT(i, j) & 0x1) << 5);
        }
    }

    g_image_queue[g_image_queue_count].data = (u_long *)g_glyph_buf;
    g_image_queue[g_image_queue_count].rect.x = x;
    g_image_queue[g_image_queue_count].rect.y = y;
    g_image_queue[g_image_queue_count].rect.w = 4;
    g_image_queue[g_image_queue_count].rect.h = 16;
    g_image_queue_count++;
}
#endif
