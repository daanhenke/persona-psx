/* Persona 1 (JP) - shared VRAM upload helper.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *   NAME @ 0x80067A34
 *   DNG  @ 0x8006FC14
 * Byte-identical once call targets and global addresses are masked out.
 */
#include <types.h>
#include <libgpu.h>

/* Uploads `rows` texture rows to VRAM, one LoadImage per row.
   The descriptor packs two shorts per word: position at +0xC, size at +0x10,
   pixel data from +0x14. A negative x or y means "take it from the descriptor
   instead of the argument". */
void UploadImageRows(void *desc, u_short x, u_short y, short rows)
{
    RECT rect;
    int *p;
    int packed;

    rect.x = x;
    rect.y = y;

    /* The redundant `p = desc` forces the base into a register before the
       add, which is how the original addresses the descriptor. */
    p = (int *)((char *)(p = desc) + 0xC);
    packed = *p++;
    if ((short)x < 0) {
        rect.x = (short)packed;
    }
    if ((short)y < 0) {
        rect.y = (short)((u_int)packed >> 16);
    }

    packed = *p++;
    rect.h = (short)((u_int)packed >> 16);
    rect.w = (short)packed;

    while (rows != 0) {
        LoadImage(&rect, (u_long *)p);
        p += 0x85;              /* one row is 0x214 bytes */
        rows--;
        rect.y++;
    }
}
