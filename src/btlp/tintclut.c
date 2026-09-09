/* Persona 1 (JP) - tinting one fighter's palette.  BTLP only.
 *   0x8008D8E8 BtlTintActorClut
 *
 * Adds a colour to every entry of an actor's palette and puts the result in
 * both the drawn set and the one it is walking toward, so the tint is there on
 * the next frame rather than faded in.
 *
 * The work is done on a copy in the scratchpad, and entry zero is left alone -
 * it is the transparent one, and lifting it would show the background through
 * a solid colour. Each channel is added at its own weight and clamped at the
 * top of its five bits, and the semi-transparency bit goes back on.
 */
#include <decomp/types.h>
#include <decomp/libc.h>

/* One actor's palette: 0x100 entries. */
#define CLUT_ENTRIES 0x100
#define CLUT_BYTES   0x200

/* Where the copy is worked on. */
#define CLUT_STAGE ((u_short *)0x1F800000)

/* A 15-bit colour, and what one unit of each channel is worth in it. */
#define CLUT_R      0x001F
#define CLUT_G      0x03E0
#define CLUT_B      0x7C00
#define CLUT_G_UNIT 0x0020
#define CLUT_B_UNIT 0x0400
#define CLUT_STP    0x8000

extern u_short *g_btl_actor_clut;
extern u_short *g_btl_actor_clut_to;
extern u_short *g_btl_actor_clut_base;

void BtlTintActorClut(int actor, int r, int g, int b)
{
    u_short *p;
    u_short  cr;
    u_short  cg;
    u_short  cb;
    u_int    sum;
    u_int    v;
    int      i;

    p = CLUT_STAGE;
    memcpy(p, g_btl_actor_clut_base + actor * (CLUT_BYTES / 2), CLUT_BYTES);

    /* Entry zero is the transparent one and is left as it came. */
    p++;
    i = 1;
    do {
        v = *p;
        sum = (v & CLUT_R) + r;
        cr = sum;
        if ((u_short)sum > CLUT_R) {
            cr = CLUT_R;
        }
        sum = (v & CLUT_G) + g * CLUT_G_UNIT;
        cg = sum;
        if ((u_short)sum > CLUT_G) {
            cg = CLUT_G;
        }
        sum = (v & CLUT_B) + b * CLUT_B_UNIT;
        cb = sum;
        if ((u_short)sum > CLUT_B) {
            cb = CLUT_B;
        }
        i++;
        *p = cr | cg | cb | CLUT_STP;
        p++;
    } while (i < CLUT_ENTRIES);

    memcpy(g_btl_actor_clut + actor * (CLUT_BYTES / 2), CLUT_STAGE, CLUT_BYTES);
    memcpy(g_btl_actor_clut_to + actor * (CLUT_BYTES / 2), CLUT_STAGE,
           CLUT_BYTES);
}
