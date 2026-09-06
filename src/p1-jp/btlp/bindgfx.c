/* Persona 1 (JP) - making a loaded graphics image usable.  BTLP only.
 *   0x80080F40 BtlBindGfx
 *
 * Everything the battle draws arrives as an image built to no fixed address,
 * with its internal pointers stored as offsets. This walks the three runs the
 * header counts and turns each offset into an address, then hangs the result
 * on one of five slots by kind.
 *
 * The caller's pointer is left just past what was consumed, so a buffer can
 * hold several images and each call binds the next one along.
 */
#include <types.h>

/* Which slot the image is bound to. */
#define GFX_MEMBER  0
#define GFX_SPECIES 1
#define GFX_EFFECT  2
#define GFX_SPARE   3
#define GFX_ACTOR   4

extern u_char *g_btl_member_gfx[];
extern u_char *g_btl_species_gfx[];
extern u_char *g_btl_effect_gfx;
extern u_char *g_btl_unused_gfx;
extern u_char *g_btl_actor_gfx;

int BtlBindGfx(u_int kind, int index, u_char **image)
{
    int     base;
    short  *counts;
    int    *p;
    int    *q;
    int    *table;
    int     v;
    int     i;
    int     j;
    u_char *start;
    int spare[5];

    /* Everything comes off the one pointer the image was loaded into - a
       separate variable for the image costs the match. */
    q = (int *)*image;
    base = (int)q;
    i = 0;
    start = (u_char *)q + 0x10;
    p = (int *)(start + *(short *)((u_char *)q + 8) * 8);
    counts = (short *)((u_char *)q + 8);

    /* The second word of each pair. */
    if (counts[1] > 0) {
        q = p + 1;
        do {
            i++;
            v = *q;
            *q = v + base;
            p += 2;
            q += 2;
        } while (i < counts[1]);
    }

    /* Then the first word of each pair. */
    j = 0;
    if (counts[2] > 0) {
        do {
            v = *p;
            *p = v + base;
            j++;
            p += 2;
        } while (j < counts[2]);
    }

    q = p;
    table = q;
    switch (kind) {
    case GFX_MEMBER:
        g_btl_member_gfx[index] = (u_char *)p;
        break;
    case GFX_SPECIES:
        g_btl_species_gfx[index] = (u_char *)p;
        break;
    case GFX_EFFECT:
        g_btl_effect_gfx = (u_char *)p;
        break;
    case GFX_SPARE:
        g_btl_unused_gfx = (u_char *)p;
        break;
    case GFX_ACTOR:
        /* The two arms are the same store, and the test is always true. It is
           there to give this case a basic block of its own, which is what puts
           the loop counter and the table pointer in the registers the original
           has them in; collapsing it costs the match. */
        if (j || q) {
            g_btl_actor_gfx = (u_char *)p;
            break;
        } else {
            g_btl_actor_gfx = (u_char *)p;
            break;
        }
    }

    /* And the flat run, which is also where the caller carries on from. */
    i = 0;
    if (counts[3] > 0) {
        p = table;
        do {
            *p = base + *q;
            q++;
            i++;
            p++;
        } while (i < counts[3]);
    }
    *image = (u_char *)q;
    return 1;
}
